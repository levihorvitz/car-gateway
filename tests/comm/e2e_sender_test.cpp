#include <cstdint>

#include <gtest/gtest.h>

#include "comm/e2e.hpp"
#include "comm/e2e_sender.hpp"
#include "support/allocation_counter.hpp"

using cvc::comm::CE2eSender;
using cvc::comm::e2eCrc;
using cvc::comm::kCounterByte;
using cvc::comm::kCounterMask;
using cvc::comm::kCrcByte;
using cvc::comm::TMessageId;
using cvc::comm::TPayload;

namespace {

constexpr TMessageId kId = 0x100;

std::uint8_t counterOf(const TPayload& payload) {
    return static_cast<std::uint8_t>(payload[kCounterByte] & kCounterMask);
}

}  // namespace

// SW_E2E_003
TEST(E2eSenderTest, CounterStartsAtZeroAndIncrements) {
    CE2eSender sender(kId);
    for (std::uint8_t expected = 0; expected < 5; ++expected) {
        TPayload payload{};
        sender.protect(payload);
        EXPECT_EQ(counterOf(payload), expected);
    }
}

// SW_E2E_003
TEST(E2eSenderTest, CounterWrapsFromFifteenToZero) {
    CE2eSender sender(kId);
    TPayload payload{};
    for (int send = 0; send < 16; ++send) {
        sender.protect(payload);
    }
    EXPECT_EQ(counterOf(payload), 15);
    sender.protect(payload);
    EXPECT_EQ(counterOf(payload), 0);
}

// SW_E2E_002
TEST(E2eSenderTest, WritesTheCrcInByteSeven) {
    CE2eSender sender(kId);
    TPayload payload{0x10, 0x27, 0x5A, 0x01, 0x00, 0x00, 0x00, 0x00};
    sender.protect(payload);
    EXPECT_EQ(payload[kCrcByte], e2eCrc(kId, payload));
}

// SW_E2E_004
TEST(E2eSenderTest, LeavesSignalBitsUnchanged) {
    CE2eSender sender(kId);
    TPayload payload{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xA0, 0x00};
    sender.protect(payload);
    EXPECT_EQ(payload[0], 0x11);
    EXPECT_EQ(payload[1], 0x22);
    EXPECT_EQ(payload[2], 0x33);
    EXPECT_EQ(payload[3], 0x44);
    EXPECT_EQ(payload[4], 0x55);
    EXPECT_EQ(payload[5], 0x66);
    EXPECT_EQ(payload[kCounterByte] & 0xF0, 0xA0);
}

// SW_E2E_010
TEST(E2eSenderTest, DoesNotAllocate) {
    CE2eSender sender(kId);
    TPayload payload{};
    const auto before = cvc::test::allocationCount();
    for (int send = 0; send < 100; ++send) {
        sender.protect(payload);
    }
    EXPECT_EQ(cvc::test::allocationCount() - before, 0u);
}
