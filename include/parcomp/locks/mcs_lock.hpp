#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include <atomic>

namespace parcomp {

class MCSLock : public ILock {
public:
    struct QNode {
        std::atomic<QNode*> next{nullptr};
        std::atomic<bool> locked{false};
    };

    MCSLock() : tail_(nullptr) {}

    void acquire(QNode& my_node) {
        my_node.next.store(nullptr, std::memory_order_relaxed);
        QNode* pred = tail_.exchange(&my_node, std::memory_order_acq_rel);
        if (pred != nullptr) {
            my_node.locked.store(true, std::memory_order_relaxed);
            pred->next.store(&my_node, std::memory_order_release);
            while (my_node.locked.load(std::memory_order_acquire)) {
            }
        }
    }

    void release(QNode& my_node) {
        if (my_node.next.load(std::memory_order_relaxed) == nullptr) {
            QNode* expected = &my_node;
            if (tail_.compare_exchange_strong(expected, nullptr, std::memory_order_release)) {
                return;
            }
            while (my_node.next.load(std::memory_order_acquire) == nullptr) {
            }
        }
        QNode* succ = my_node.next.load(std::memory_order_relaxed);
        succ->locked.store(false, std::memory_order_release);
    }

    void acquire() override {
        acquire(get_thread_local_node());
    }

    void release() override {
        release(get_thread_local_node());
    }

private:
    static QNode& get_thread_local_node() {
        thread_local QNode local_node;
        return local_node;
    }

    std::atomic<QNode*> tail_;
};

} // namespace parcomp

