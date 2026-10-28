cmake_minimum_required(VERSION 3.20)
project(ParallelComputingAssignment1
    VERSION 1.0.0
    DESCRIPTION "Assignment 1: Mutex Locks and Benchmarks"
    LANGUAGES CXX
)

# Standard C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs (clangd, vscode, antigravity)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(Threads REQUIRED)

# Core Library: parcomp
add_subdirectory(src)

# Tests
enable_testing()
add_subdirectory(tests)

# Benchmarks
add_subdirectory(benchmarks)
