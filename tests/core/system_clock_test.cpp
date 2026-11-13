#include <chrono>

#include <gtest/gtest.h>

#include "core/system_clock.hpp"

using namespace std::chrono_literals;
using gateway::core::CSystemClock;

TEST(CSystemClock, NeverGoesBackwards) {
    CSystemClock clock;
    const auto first = clock.now();
    const auto second = clock.now();
    EXPECT_LE(first, second);
}

TEST(CSystemClock, SleepUntilWaitsForDeadline) {
    CSystemClock clock;
    const auto deadline = clock.now() + 5ms;
    clock.sleepUntil(deadline);
    EXPECT_GE(clock.now(), deadline);
}
