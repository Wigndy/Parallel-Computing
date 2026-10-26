#include "parcomp/parcomp.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <cassert>
#include <chrono>
#include <atomic>

template <typename LockType>
void test_mutex_correctness(const std::string& name, size_t num_threads = 8, size_t iters_per_thread = 20000) {
    std::cout << "[Test] " << name << " (" << num_threads << " threads, "
              << iters_per_thread << " iters/thread)... " << std::flush;


    LockType lock;
    int64_t shared_counter = 0;
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back([&]() {
            for (size_t i = 0; i < iters_per_thread; ++i) {
                lock.acquire();
                ++shared_counter;
                lock.release();
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    int64_t expected = static_cast<int64_t>(num_threads * iters_per_thread);
    if (shared_counter != expected) {
        std::cerr << " FAILED! Expected: " << expected << ", got: " << shared_counter << std::endl;
        std::exit(1);
    }
    std::cout << " PASSED (counter = " << shared_counter << ")" << std::endl;
}

void test_tryout_lock() {
    std::cout << "[Test] TryoutLock... " << std::flush;

    parcomp::TryoutLock lock;
    assert(lock.try_acquire() == true);
    assert(lock.try_acquire() == false);
    lock.release();
    assert(lock.try_acquire() == true);
    lock.release();

    lock.acquire();
    std::atomic<bool> timed_out{false};
    std::thread t1([&]() {
        auto start = std::chrono::steady_clock::now();
        bool ok = lock.try_acquire_for(std::chrono::milliseconds(50));
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        assert(!ok);
        assert(elapsed.count() >= 45);
        (void)ok;
        (void)elapsed;
        timed_out.store(true);
    });

    t1.join();
    assert(timed_out.load());
    lock.release();

    lock.acquire();
    std::atomic<bool> acquired_in_time{false};
    std::thread t2([&]() {
        bool ok = lock.try_acquire_for(std::chrono::milliseconds(200));
        assert(ok);
        (void)ok;
        acquired_in_time.store(true);
        lock.release();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    lock.release();
    t2.join();
    assert(acquired_in_time.load());

    std::cout << "PASSED" << std::endl;
}

void test_recursive_lock() {
    std::cout << "[Test] RecursiveLock... " << std::flush;

    parcomp::RecursiveLock rec_lock;
    assert(rec_lock.recursion_count() == 0);
    rec_lock.acquire();
    assert(rec_lock.recursion_count() == 1);
    rec_lock.acquire();
    assert(rec_lock.recursion_count() == 2);
    rec_lock.acquire();
    assert(rec_lock.recursion_count() == 3);

    std::atomic<bool> other_acquired{false};
    std::thread t([&]() {
        other_acquired.store(rec_lock.try_acquire());
    });
    t.join();
    assert(other_acquired.load() == false);

    rec_lock.release();
    assert(rec_lock.recursion_count() == 2);
    rec_lock.release();
    assert(rec_lock.recursion_count() == 1);
    rec_lock.release();
    assert(rec_lock.recursion_count() == 0);

    std::thread t2([&]() {
        other_acquired.store(rec_lock.try_acquire());
        if (other_acquired.load()) {
            rec_lock.release();
        }
    });
    t2.join();
    assert(other_acquired.load() == true);

    std::cout << "PASSED" << std::endl;
}

void test_rw_lock() {
    std::cout << "[Test] RWLock... " << std::flush;

    parcomp::RWLock rw_lock;
    std::atomic<int> active_readers{0};
    std::atomic<bool> max_readers_verified{false};

    std::vector<std::thread> readers;
    for (int i = 0; i < 4; ++i) {
        readers.emplace_back([&]() {
            rw_lock.acquire_read();
            int current = ++active_readers;
            if (current >= 3) {
                max_readers_verified.store(true);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            --active_readers;
            rw_lock.release_read();
        });
    }

    for (auto& r : readers) {
        r.join();
    }
    assert(max_readers_verified.load());
    assert(active_readers.load() == 0);

    int shared_data = 100;
    rw_lock.acquire_write();
    std::thread reader_during_write([&]() {
        rw_lock.acquire_read();
        assert(shared_data == 200);
        (void)shared_data;
        rw_lock.release_read();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    shared_data = 200;
    rw_lock.release_write();
    reader_during_write.join();

    std::cout << "PASSED" << std::endl;
}

void test_banking_deadlock_avoidance() {
    std::cout << "[Test] Banking Simulation... " << std::flush;

    parcomp::BankingBenchmarkConfig cfg;
    cfg.num_threads = 4;
    cfg.num_accounts = 10;
    cfg.num_ops_per_thread = 500;
    cfg.timeout_us = 500;
    cfg.read_ratio = 0.5;

    auto res = parcomp::run_banking_simulation(cfg);
    assert(res.total_transfers > 0);
    assert(res.successful_transfers > 0);
    assert(res.read_queries > 0);
    assert(res.elapsed_ms > 0.0);

    std::cout << "PASSED (" << res.successful_transfers << " txs ok, "
              << res.aborted_transfers << " timeouts, 0 deadlocks)" << std::endl;
}

int main() {
    test_mutex_correctness<parcomp::StdMutexLock>("StdMutexLock");
    test_mutex_correctness<parcomp::TASLock>("TASLock");
    test_mutex_correctness<parcomp::TTASLock>("TTASLock");
    test_mutex_correctness<parcomp::TTASBackoffLock>("TTASBackoffLock");
    test_mutex_correctness<parcomp::TicketLock>("TicketLock");
    test_mutex_correctness<parcomp::CLHLock>("CLHLock");
    test_mutex_correctness<parcomp::MCSLock>("MCSLock");

    test_tryout_lock();
    test_recursive_lock();
    test_rw_lock();
    test_banking_deadlock_avoidance();

    std::cout << "All tests passed successfully.\n";
    return 0;
}

