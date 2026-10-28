# tests/CMakeLists.txt

add_executable(test_locks test_locks.cpp)
target_link_libraries(test_locks PRIVATE parcomp)
add_test(NAME test_locks COMMAND test_locks)
