#pragma once

#include <chrono>

#include "core/clock.hpp"

namespace cvc::app {

constexpr core::TDuration kCyclePeriod = std::chrono::milliseconds(20);  // SW_SCHED_001

}  // namespace cvc::app
