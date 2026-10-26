#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include <atomic>
#include <cstdint>

namespace parcomp {

class TicketLock : public ILock {
public:
    TicketLock() : ticket_(0), now_serving_(0) {}

    void acquire() override {
        uint64_t my_ticket = ticket_.fetch_add(1, std::memory_order_relaxed);
        while (now_serving_.load(std::memory_order_acquire) != my_ticket) {
        }
    }

    void release() override {
        now_serving_.fetch_add(1, std::memory_order_release);
    }

private:
    std::atomic<uint64_t> ticket_;
    std::atomic<uint64_t> now_serving_;
};

} // namespace parcomp
