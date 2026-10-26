#include "parcomp/parcomp.hpp"
#include <iostream>
#include <string>
#include <iomanip>

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "Options:\n"
              << "  --threads <N>       Number of worker threads (default: 4)\n"
              << "  --accounts <N>      Number of bank accounts in system (default: 50)\n"
              << "  --ops <N>           Number of operations per thread (default: 5000)\n"
              << "  --timeout-us <N>    Timeout budget in microseconds for TryoutLock (default: 1000)\n"
              << "  --read-ratio <R>    Ratio of read queries vs transfers [0.0 - 1.0] (default: 0.8)\n"
              << "  --help              Display this help message\n";
}

int main(int argc, char** argv) {
    parcomp::BankingBenchmarkConfig cfg;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--threads" && i + 1 < argc) {
            cfg.num_threads = std::stoul(argv[++i]);
        } else if (arg == "--accounts" && i + 1 < argc) {
            cfg.num_accounts = std::stoul(argv[++i]);
        } else if (arg == "--ops" && i + 1 < argc) {
            cfg.num_ops_per_thread = std::stoul(argv[++i]);
        } else if (arg == "--timeout-us" && i + 1 < argc) {
            cfg.timeout_us = static_cast<unsigned>(std::stoul(argv[++i]));
        } else if (arg == "--read-ratio" && i + 1 < argc) {
            cfg.read_ratio = std::stod(argv[++i]);
        } else if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
    }

    std::cout << "--- Banking Simulation Benchmark ---\n"
              << "Threads: " << cfg.num_threads << ", Accounts: " << cfg.num_accounts
              << ", Ops/thread: " << cfg.num_ops_per_thread << ", Timeout: " << cfg.timeout_us
              << " us, ReadRatio: " << std::fixed << std::setprecision(1) << (cfg.read_ratio * 100.0) << "%\n";

    auto res = parcomp::run_banking_simulation(cfg);

    std::cout << "Results:\n"
              << "  Total Ops: " << (res.total_transfers + res.read_queries) << "\n"
              << "  Successful Transfers: " << res.successful_transfers << "\n"
              << "  Aborted Transfers: " << res.aborted_transfers << " (" << std::fixed << std::setprecision(2) << res.abort_rate_percent << "%)\n"
              << "  Read Queries: " << res.read_queries << "\n"
              << "  Time: " << std::fixed << std::setprecision(2) << res.elapsed_ms << " ms\n"
              << "  Throughput: " << std::fixed << std::setprecision(2) << res.throughput_ops_sec << " ops/sec\n";


    return 0;
}
