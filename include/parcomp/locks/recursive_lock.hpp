#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include <atomic>
#include <thread>
#include <cstdint>
#include <chrono>
#include <stdexcept>

namespace parcomp {

class RecursiveLock : public ILock {
public:
    RecursiveLock() : owner_(std::thread::id{}), count_(0) {}

    void acquire() override {
        const std::thread::id self = std::this_thread::get_id();
        if (self == owner_.load(std::memory_order_relaxed)) {
            count_++;
            return;
        }

        std::thread::id expected{};
        while (!owner_.compare_exchange_weak(expected, self, std::memory_order_acquire, std::memory_order_relaxed)) {
            expected = std::thread::id{};
        }
        count_ = 1;
    }

    void release() override {
        const std::thread::id self = std::this_thread::get_id();
        if (self != owner_.load(std::memory_order_relaxed)) {
            throw std::logic_error("Illegal monitor state: caller thread does not own the lock.");
        }

        count_--;
        if (count_ == 0) {
            owner_.store(std::thread::id{}, std::memory_order_release);
        }
    }

    bool try_acquire() {
        const std::thread::id self = std::this_thread::get_id();
        if (self == owner_.load(std::memory_order_relaxed)) {
            count_++;
            return true;
        }

        std::thread::id expected{};
        if (owner_.compare_exchange_strong(expected, self, std::memory_order_acquire, std::memory_order_relaxed)) {
            count_ = 1;
            return true;
        }
        return false;
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

    [[nodiscard]] uint32_t recursion_count() const {
        return count_;
    }

    [[nodiscard]] std::thread::id owner() const {
        return owner_.load(std::memory_order_relaxed);
    }

private:
    std::atomic<std::thread::id> owner_;
    uint32_t count_;
};

} // namespace parcomp
