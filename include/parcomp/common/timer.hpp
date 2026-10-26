#pragma once

#include <chrono>

namespace parcomp {

/**
 * @brief High-precision timer for measuring execution time and benchmark metrics.
 */
class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    Timer() : start_time_(Clock::now()) {}

    void reset() {
        start_time_ = Clock::now();
    }

    /**
     * @brief Elapsed time in seconds (floating point).
     */
    [[nodiscard]] double elapsed_sec() const {
        auto now = Clock::now();
        std::chrono::duration<double> diff = now - start_time_;
        return diff.count();
    }

    /**
     * @brief Elapsed time in milliseconds (floating point).
     */
    [[nodiscard]] double elapsed_ms() const {
        return elapsed_sec() * 1000.0;
    }

    /**
     * @brief Elapsed time in microseconds (floating point).
     */
    [[nodiscard]] double elapsed_us() const {
        return elapsed_sec() * 1000000.0;
    }

    /**
     * @brief Elapsed time in nanoseconds (integer).
     */
    [[nodiscard]] long long elapsed_ns() const {
        auto now = Clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_).count();
    }

private:
    TimePoint start_time_;
};

} // namespace parcomp
