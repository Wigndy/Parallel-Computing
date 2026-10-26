#pragma once

namespace parcomp {

/**
 * @brief Base interface for mutual exclusion locks.
 * Supports both:
 * 1. Assignment convention: acquire() / release()
 * 2. C++ BasicLockable concept: lock() / unlock() (allowing std::lock_guard)
 */
class ILock {
public:
    virtual ~ILock() = default;

    virtual void acquire() = 0;
    virtual void release() = 0;

    // C++ Lockable adapters
    void lock() { acquire(); }
    void unlock() { release(); }
};

} // namespace parcomp
