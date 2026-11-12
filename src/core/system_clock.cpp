#include "core/system_clock.hpp"

#include <thread>

namespace gateway::core {

TimePoint SystemClock::now() const {
    return std::chrono::steady_clock::now();
}

void SystemClock::sleepUntil(TimePoint deadline) {
    std::this_thread::sleep_until(deadline);
}

}  // namespace gateway::core
