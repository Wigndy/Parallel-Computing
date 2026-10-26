#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include <atomic>
#include <chrono>

namespace parcomp {

class TryoutLock : public ILock {
public:
    TryoutLock() : state_(false) {}

    void acquire() override {
        while (!try_acquire()) {
            while (state_.load(std::memory_order_relaxed)) {
            }
        }
    }

    void release() override {
        state_.store(false, std::memory_order_release);
    }

    bool try_acquire() {
        bool expected = false;
        return state_.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed);
    }

    template <typename Rep, typename Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& timeout) {
        if (try_acquire()) {
            return true;
        }

        const auto start = std::chrono::steady_clock::now();
        while (true) {
            if (try_acquire()) {
                return true;
            }
            if ((std::chrono::steady_clock::now() - start) >= timeout) {
                return false;
            }
        }
    }

private:
    std::atomic<bool> state_;
};

} // namespace parcomp
