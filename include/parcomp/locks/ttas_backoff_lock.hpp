#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include "parcomp/common/backoff.hpp"
#include <atomic>

namespace parcomp {

class TTASBackoffLock : public ILock {
public:
    explicit TTASBackoffLock(unsigned min_delay_ns = 10, unsigned max_delay_ns = 10000)
        : state_(false), min_delay_(min_delay_ns), max_delay_(max_delay_ns) {}

    void acquire() override {
        ExponentialBackoff backoff(min_delay_, max_delay_);
        while (true) {
            while (state_.load(std::memory_order_relaxed)) {
            }
            if (!state_.exchange(true, std::memory_order_acquire)) {
                return;
            }
            backoff.backoff();
        }
    }

    void release() override {
        state_.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> state_;
    unsigned min_delay_;
    unsigned max_delay_;
};

} // namespace parcomp

