#include "parcomp/common/banking_workload.hpp"
#include "parcomp/common/timer.hpp"
#include <thread>
#include <random>
#include <vector>
#include <atomic>

namespace parcomp {

BankAccount::BankAccount(uint32_t id, double initial_balance)
    : id_(id), balance_(initial_balance) {}

void BankAccount::charge_fee(double fee) {
    // Inner method acquiring rec_lock
    rec_lock.acquire();
    balance_ -= fee;
    rec_lock.release();
}

bool BankAccount::withdraw_with_fee(double amount, double fee) {
    // Outer method acquiring rec_lock and invoking charge_fee() which re-acquires rec_lock
    rec_lock.acquire();
    bool success = false;
    if (balance_ >= amount + fee) {
        balance_ -= amount;
        // Nested call on same thread: re-acquires rec_lock cleanly without self-deadlock
        charge_fee(fee);
        success = true;
    }
    rec_lock.release();
    return success;
}

void BankAccount::deposit(double amount) {
    rec_lock.acquire();
    balance_ += amount;
    rec_lock.release();
}

double BankAccount::get_balance_shared() {
    rw_lock.acquire_read();
    double b = balance_;
    rw_lock.release_read();
    return b;
}

void BankAccount::update_balance_exclusive(double delta) {
    rw_lock.acquire_write();
    balance_ += delta;
    rw_lock.release_write();
}

bool transfer_with_timeout(BankAccount& from,
                           BankAccount& to,
                           double amount,
                           std::chrono::microseconds timeout,
                           double fee) {
    if (from.id() == to.id()) {
        return false;
    }

    const auto start_time = std::chrono::steady_clock::now();

    // 1. Acquire source account try_lock with timeout
    if (!from.try_lock.try_acquire_for(timeout)) {
        return false; // Timed out waiting for source account
    }

    // 2. Calculate remaining budget for destination account
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start_time);
    if (elapsed >= timeout) {
        from.try_lock.release();
        return false;
    }

    auto remaining_timeout = timeout - elapsed;

    // 3. Attempt to acquire destination try_lock within remaining time
    // Bounded wait prevents classic Dining Philosophers deadlock!
    if (!to.try_lock.try_acquire_for(remaining_timeout)) {
        // Contention detected / Timed out: MUST release source lock to prevent deadlock
        from.try_lock.release();
        return false;
    }

    // 4. Critical Section: Perform transfer with nested fee deduction
    bool success = from.withdraw_with_fee(amount, fee);
    if (success) {
        to.deposit(amount);
    }

    // 5. Release locks
    to.try_lock.release();
    from.try_lock.release();

    return success;
}

BankingBenchmarkResult run_banking_simulation(const BankingBenchmarkConfig& cfg) {
    std::vector<std::unique_ptr<BankAccount>> accounts;
    accounts.reserve(cfg.num_accounts);
    for (size_t i = 0; i < cfg.num_accounts; ++i) {
        accounts.push_back(std::make_unique<BankAccount>(static_cast<uint32_t>(i), 100000.0));
    }

    std::vector<std::thread> workers;
    workers.reserve(cfg.num_threads);

    std::atomic<size_t> total_transfers{0};
    std::atomic<size_t> successful_transfers{0};
    std::atomic<size_t> aborted_transfers{0};
    std::atomic<size_t> read_queries{0};

    Timer timer;
    timer.reset();

    for (size_t t = 0; t < cfg.num_threads; ++t) {
        workers.emplace_back([&, t]() {
            std::mt19937_64 rng(1337 + t);
            std::uniform_real_distribution<double> dist_op(0.0, 1.0);
            std::uniform_int_distribution<size_t> dist_acc(0, cfg.num_accounts - 1);
            std::uniform_real_distribution<double> dist_amount(1.0, 100.0);

            for (size_t op = 0; op < cfg.num_ops_per_thread; ++op) {
                double op_type = dist_op(rng);
                if (op_type < cfg.read_ratio) {
                    // Shared read query (Portfolio / Balance check)
                    size_t acc_idx = dist_acc(rng);
                    double b = accounts[acc_idx]->get_balance_shared();
                    (void)b;
                    read_queries.fetch_add(1, std::memory_order_relaxed);
                } else {
                    // Cross-account transfer (potential Dining Philosophers contention)
                    size_t from_idx = dist_acc(rng);
                    size_t to_idx = dist_acc(rng);
                    while (to_idx == from_idx && cfg.num_accounts > 1) {
                        to_idx = dist_acc(rng);
                    }

                    double amount = dist_amount(rng);
                    total_transfers.fetch_add(1, std::memory_order_relaxed);

                    bool ok = transfer_with_timeout(
                        *accounts[from_idx],
                        *accounts[to_idx],
                        amount,
                        std::chrono::microseconds(cfg.timeout_us),
                        0.5
                    );

                    if (ok) {
                        successful_transfers.fetch_add(1, std::memory_order_relaxed);
                    } else {
                        aborted_transfers.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            }
        });
    }

    for (auto& th : workers) {
        th.join();
    }

    double elapsed_ms = timer.elapsed_ms();

    BankingBenchmarkResult res;
    res.elapsed_ms = elapsed_ms;
    res.total_transfers = total_transfers.load();
    res.successful_transfers = successful_transfers.load();
    res.aborted_transfers = aborted_transfers.load();
    res.read_queries = read_queries.load();

    size_t total_ops = res.total_transfers + res.read_queries;
    res.throughput_ops_sec = (elapsed_ms > 0.0) ? (static_cast<double>(total_ops) / (elapsed_ms / 1000.0)) : 0.0;
    res.abort_rate_percent = (res.total_transfers > 0)
        ? (100.0 * static_cast<double>(res.aborted_transfers) / static_cast<double>(res.total_transfers))
        : 0.0;

    return res;
}

} // namespace parcomp
