#pragma once

#include "parcomp/locks/lock_interface.hpp"
#include <atomic>

namespace parcomp {

class CLHLock : public ILock {
public:
    struct QNode {
        std::atomic<bool> succ_must_wait{true};
    };

    CLHLock() {
        initial_node_ = new QNode();
        initial_node_->succ_must_wait.store(false, std::memory_order_relaxed);
        tail_.store(initial_node_, std::memory_order_relaxed);
    }

    ~CLHLock() override {
        delete tail_.load(std::memory_order_relaxed);
    }

    CLHLock(const CLHLock&) = delete;
    CLHLock& operator=(const CLHLock&) = delete;

    void acquire() override {
        QNode* node = get_thread_node();
        node->succ_must_wait.store(true, std::memory_order_relaxed);

        QNode* pred = tail_.exchange(node, std::memory_order_acq_rel);
        get_thread_pred() = pred;

        while (pred->succ_must_wait.load(std::memory_order_acquire)) {
        }
    }

    void release() override {
        QNode* node = get_thread_node();
        node->succ_must_wait.store(false, std::memory_order_release);
        get_thread_node() = get_thread_pred();
    }

private:
    static QNode*& get_thread_node() {
        thread_local QNode* node = new QNode();
        return node;
    }

    static QNode*& get_thread_pred() {
        thread_local QNode* pred = nullptr;
        return pred;
    }

    std::atomic<QNode*> tail_;
    QNode* initial_node_;
};

} // namespace parcomp
