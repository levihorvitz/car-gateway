#pragma once

#include <chrono>

#include "core/clock.hpp"

namespace gateway::app {

constexpr core::TDuration kCyclePeriod = std::chrono::milliseconds(20);  // SW_SCHED_001

}  // namespace gateway::app
