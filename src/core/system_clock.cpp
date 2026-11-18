#include "core/system_clock.hpp"

#include <thread>

namespace cvc::core {

TTimePoint CSystemClock::now() const {
    return std::chrono::steady_clock::now();
}

void CSystemClock::sleepUntil(TTimePoint deadline) {
    std::this_thread::sleep_until(deadline);
}

}  // namespace cvc::core
