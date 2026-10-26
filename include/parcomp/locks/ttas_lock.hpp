#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include <atomic>

namespace parcomp {

class TTASLock : public ILock {
public:
    TTASLock() : state_(false) {}

    void acquire() override {
        while (true) {
            while (state_.load(std::memory_order_relaxed)) {
            }
            if (!state_.exchange(true, std::memory_order_acquire)) {
                return;
            }
        }
    }

    void release() override {
        state_.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> state_;
};

} // namespace parcomp

