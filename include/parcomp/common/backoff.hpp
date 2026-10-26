#pragma once

#include <chrono>
#include <thread>
#include <algorithm>

namespace parcomp {

class ExponentialBackoff {
public:
    explicit ExponentialBackoff(unsigned min_delay_ns = 10, unsigned max_delay_ns = 10000)
        : min_delay_(min_delay_ns), max_delay_(max_delay_ns), current_delay_(min_delay_ns) {}

    void backoff() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(current_delay_));
        if (current_delay_ < max_delay_) {
            current_delay_ = std::min(current_delay_ * 2, max_delay_);
        }
    }

    void reset() {
        current_delay_ = min_delay_;
    }

    [[nodiscard]] unsigned current_delay() const { return current_delay_; }
    [[nodiscard]] unsigned max_delay() const { return max_delay_; }

private:
    unsigned min_delay_;
    unsigned max_delay_;
    unsigned current_delay_;
};

} // namespace parcomp

