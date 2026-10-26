#pragma once

#include "parcomp/locks/tryout_lock.hpp"
#include "parcomp/locks/recursive_lock.hpp"
#include "parcomp/locks/rw_lock.hpp"
#include <vector>
#include <chrono>
#include <cstdint>
#include <string>

namespace parcomp {

/**
 * @brief BankAccount model demonstrating Real-time Financial Concurrency.
 * Combines TryoutLock (timed transfer), RecursiveLock (nested fees), and RWLock (read-heavy queries).
 */
class BankAccount {
public:
    explicit BankAccount(uint32_t id = 0, double initial_balance = 10000.0);

    [[nodiscard]] uint32_t id() const noexcept { return id_; }
    [[nodiscard]] double balance() const noexcept { return balance_; }

    // Nested operations demonstrating RecursiveLock
    void charge_fee(double fee);
    bool withdraw_with_fee(double amount, double fee);
    void deposit(double amount);

    // Read-heavy operations demonstrating RWLock
    double get_balance_shared();
    void update_balance_exclusive(double delta);

    // Locks protecting different aspects of the account
    TryoutLock try_lock;
    RecursiveLock rec_lock;
    RWLock rw_lock;

private:
    uint32_t id_;
    double balance_;
};

/**
 * @brief Performs cross-account money transfer with bounded timeout to eliminate
 * classic Dining Philosophers deadlocks (e.g. Account A -> B vs Account B -> A).
 * 
 * @param from Source bank account
 * @param to Destination bank account
 * @param amount Amount to transfer
 * @param timeout Maximum duration to wait before aborting to avoid deadlock
 * @param fee Transaction fee deducted inside via nested recursive call
 * @return true if completed within timeout, false if aborted due to contention
 */
bool transfer_with_timeout(BankAccount& from,
                           BankAccount& to,
                           double amount,
                           std::chrono::microseconds timeout,
                           double fee = 1.0);

/**
 * @brief Configuration for the Real-time Banking Benchmark.
 */
struct BankingBenchmarkConfig {
    size_t num_threads = 4;
    size_t num_accounts = 50;
    size_t num_ops_per_thread = 5000;
    unsigned timeout_us = 1000;      // 1ms bounded wait
    double read_ratio = 0.8;         // 80% balance inquiries, 20% transfers
};

/**
 * @brief Performance and reliability metrics for the Banking Benchmark.
 */
struct BankingBenchmarkResult {
    double elapsed_ms = 0.0;
    size_t total_transfers = 0;
    size_t successful_transfers = 0;
    size_t aborted_transfers = 0;
    size_t read_queries = 0;
    double throughput_ops_sec = 0.0;
    double abort_rate_percent = 0.0;
};

/**
 * @brief Executes the concurrent banking simulation across worker threads.
 */
BankingBenchmarkResult run_banking_simulation(const BankingBenchmarkConfig& cfg);

} // namespace parcomp
