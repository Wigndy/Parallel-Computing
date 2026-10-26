#include "parcomp/parcomp.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <iomanip>

struct BenchmarkConfig {
    size_t num_threads = 4;
    size_t num_iters = 10000;
    size_t outside_work = 1;
    int prime_n = 10007;
    std::string lock_type = "all";
};

template <typename LockType>
double run_lock_benchmark(const std::string& name, const BenchmarkConfig& cfg) {
    LockType lock;
    parcomp::Timer timer;

    std::vector<std::thread> threads;
    threads.reserve(cfg.num_threads);

    timer.reset();

    for (size_t t = 0; t < cfg.num_threads; ++t) {
        threads.emplace_back([&]() {
            // Child thread's code matching assignment PDF fragment (page 1)
            for (size_t i = 0; i < cfg.num_iters; ++i) {
                lock.acquire();

                // Work inside critical section
                parcomp::is_prime(cfg.prime_n);

                lock.release();

                // Work outside critical section (determines contention)
                for (size_t j = 0; j < cfg.outside_work; ++j) {
                    parcomp::is_prime(cfg.prime_n);
                }
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    double elapsed_ms = timer.elapsed_ms();
    std::cout << std::left << std::setw(22) << name
              << " | Threads: " << std::setw(3) << cfg.num_threads
              << " | Iters: " << std::setw(7) << cfg.num_iters
              << " | OutsideWork: " << std::setw(2) << cfg.outside_work
              << " | Time: " << std::fixed << std::setprecision(2) << std::setw(8) << elapsed_ms << " ms"
              << std::endl;
    return elapsed_ms;
}

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "Options:\n"
              << "  --threads <N>       Number of worker threads (default: 4)\n"
              << "  --iters <N>         Number of iterations per thread (default: 10000)\n"
              << "  --outside-work <N>  Amount of work outside critical section (default: 1)\n"
              << "  --prime-n <N>       Number to test for primality (default: 10007)\n"
              << "  --lock <type>       tas | ttas | ttas_backoff | ticket | clh | mcs | std | all (default: all)\n"
              << "  --help              Display this help message\n";
}

int main(int argc, char** argv) {
    BenchmarkConfig cfg;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--threads" && i + 1 < argc) {
            cfg.num_threads = std::stoul(argv[++i]);
        } else if (arg == "--iters" && i + 1 < argc) {
            cfg.num_iters = std::stoul(argv[++i]);
        } else if (arg == "--outside-work" && i + 1 < argc) {
            cfg.outside_work = std::stoul(argv[++i]);
        } else if (arg == "--prime-n" && i + 1 < argc) {
            cfg.prime_n = std::stoi(argv[++i]);
        } else if (arg == "--lock" && i + 1 < argc) {
            cfg.lock_type = argv[++i];
        } else if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
    }

    std::cout << "--- Mutex Locks Benchmark (is_prime) ---\n";


    if (cfg.lock_type == "tas" || cfg.lock_type == "all") {
        run_lock_benchmark<parcomp::TASLock>("TASLock", cfg);
    }
    if (cfg.lock_type == "ttas" || cfg.lock_type == "all") {
        run_lock_benchmark<parcomp::TTASLock>("TTASLock", cfg);
    }
    if (cfg.lock_type == "ttas_backoff" || cfg.lock_type == "all") {
        run_lock_benchmark<parcomp::TTASBackoffLock>("TTASBackoffLock", cfg);
    }
    if (cfg.lock_type == "ticket" || cfg.lock_type == "all") {
        run_lock_benchmark<parcomp::TicketLock>("TicketLock (FIFO)", cfg);
    }
    if (cfg.lock_type == "clh" || cfg.lock_type == "all") {
        run_lock_benchmark<parcomp::CLHLock>("CLHLock (Queue)", cfg);
    }
    if (cfg.lock_type == "mcs" || cfg.lock_type == "all") {
        run_lock_benchmark<parcomp::MCSLock>("MCSLock (Queue)", cfg);
    }
    if (cfg.lock_type == "std" || cfg.lock_type == "all") {
        run_lock_benchmark<parcomp::StdMutexLock>("StdMutexLock (baseline)", cfg);
    }

    return 0;
}
