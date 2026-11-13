#include <chrono>

#include <gtest/gtest.h>

#include "fakes/fake_clock.hpp"

using namespace std::chrono_literals;
using gateway::test::CFakeClock;

TEST(CFakeClock, StartsAtZero) {
    CFakeClock clock;
    EXPECT_EQ(clock.now().time_since_epoch(), 0ns);
}

TEST(CFakeClock, AdvanceMovesTimeForward) {
    CFakeClock clock;
    clock.advance(20ms);
    EXPECT_EQ(clock.now().time_since_epoch(), 20ms);
}

TEST(CFakeClock, SleepUntilJumpsToDeadline) {
    CFakeClock clock;
    const auto deadline = clock.now() + 50ms;
    clock.sleepUntil(deadline);
    EXPECT_EQ(clock.now(), deadline);
}

TEST(CFakeClock, SleepUntilPastDeadlineDoesNotGoBack) {
    CFakeClock clock;
    clock.advance(100ms);
    const auto before = clock.now();
    clock.sleepUntil(before - 30ms);
    EXPECT_EQ(clock.now(), before);
}
