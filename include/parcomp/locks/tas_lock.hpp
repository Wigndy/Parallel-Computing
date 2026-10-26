#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include <atomic>

namespace parcomp {

class TASLock : public ILock {
public:
    TASLock() {
        state_.clear();
    }

    void acquire() override {
        while (state_.test_and_set(std::memory_order_acquire)) {
        }
    }

    void release() override {
        state_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag state_ = ATOMIC_FLAG_INIT;
};

} // namespace parcomp
