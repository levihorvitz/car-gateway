#include <chrono>

#include <gtest/gtest.h>

#include "core/system_clock.hpp"

using namespace std::chrono_literals;
using gateway::core::SystemClock;

TEST(SystemClock, NeverGoesBackwards) {
    SystemClock clock;
    const auto first = clock.now();
    const auto second = clock.now();
    EXPECT_LE(first, second);
}

TEST(SystemClock, SleepUntilWaitsForDeadline) {
    SystemClock clock;
    const auto deadline = clock.now() + 5ms;
    clock.sleepUntil(deadline);
    EXPECT_GE(clock.now(), deadline);
}
