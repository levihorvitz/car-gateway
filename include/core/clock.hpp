#pragma once

#include <chrono>

namespace gateway::core {

// A point in time on the monotonic clock (not affected by wall-clock changes).
using TimePoint = std::chrono::steady_clock::time_point;
// A length of time, in nanoseconds.
using Duration = std::chrono::nanoseconds;

// Time source interface; lets tests replace real time with a fake clock.
class IClock {
public:
    virtual ~IClock() = default;

    // Returns the current time.
    [[nodiscard]] virtual TimePoint now() const = 0;
    // Blocks until the given absolute time.
    virtual void sleepUntil(TimePoint deadline) = 0;
};

// Production clock backed by std::chrono::steady_clock.

}  // namespace gateway::core
