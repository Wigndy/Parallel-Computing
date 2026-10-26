#pragma once

#include <atomic>
#include <cstdint>

namespace parcomp {

class RWLock {
public:
    RWLock() : state_(0) {}

    void acquire_read() {
        int32_t expected = state_.load(std::memory_order_relaxed);
        while (true) {
            if (expected < 0) {
                // A writer currently holds the lock; spin wait
                expected = state_.load(std::memory_order_relaxed);
                continue;
            }
            if (state_.compare_exchange_weak(expected, expected + 1, std::memory_order_acquire, std::memory_order_relaxed)) {
                break;
            }
        }
    }

    void release_read() {
        state_.fetch_sub(1, std::memory_order_release);
    }

    void acquire_write() {
        while (true) {
            int32_t expected = 0;
            if (state_.load(std::memory_order_relaxed) == 0 &&
                state_.compare_exchange_weak(expected, -1, std::memory_order_acquire, std::memory_order_relaxed)) {
                break;
            }
        }
    }

    void release_write() {
        state_.store(0, std::memory_order_release);
    }

private:
    std::atomic<int32_t> state_;
};

} // namespace parcomp
