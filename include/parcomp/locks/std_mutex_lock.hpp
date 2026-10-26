#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include <mutex>

namespace parcomp {

/**
 * @brief Adapter wrapping std::mutex to conform to ILock.
 * Used as a baseline for performance comparisons in Assignment 1.
 */
class StdMutexLock : public ILock {
public:
    void acquire() override {
        mutex_.lock();
    }

    void release() override {
        mutex_.unlock();
    }

private:
    std::mutex mutex_;
};

} // namespace parcomp
