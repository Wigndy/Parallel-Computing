# benchmarks/CMakeLists.txt

# Problem 1: Mutex Locks Benchmark (is_prime)
add_executable(bench_locks bench_prime_locks.cpp)
target_link_libraries(bench_locks PRIVATE parcomp)
set_project_warnings(bench_locks)

add_executable(bench_prime_locks bench_prime_locks.cpp)
target_link_libraries(bench_prime_locks PRIVATE parcomp)
set_project_warnings(bench_prime_locks)

# Problem 2: Realtime Banking & Deadlock Avoidance Benchmark
add_executable(bench_banking_locks bench_banking_locks.cpp)
target_link_libraries(bench_banking_locks PRIVATE parcomp)
set_project_warnings(bench_banking_locks)

# Output binaries directly into build/bin for convenience
set_target_properties(
    bench_locks
    bench_prime_locks
    bench_banking_locks
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
