#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include "comm/crc8.hpp"
#include "comm/e2e.hpp"
#include "comm/e2e_receiver.hpp"
#include "support/allocation_counter.hpp"

using cvc::comm::CE2eReceiver;
using cvc::comm::crc8SaeJ1850;
using cvc::comm::EE2eResult;
using cvc::comm::kCounterByte;
using cvc::comm::kCrcByte;
using cvc::comm::TMessageId;
using cvc::comm::TPayload;

namespace {

constexpr TMessageId kId = 0x110;

// Builds a frame independently of the code under test: counter in byte 6, CRC-8 over ID and bytes 0-6 in byte 7.
TPayload frame(std::uint8_t counter, std::uint8_t signal = 0x2A, TMessageId id = kId) {
    TPayload payload{signal, 0x01, 0x02, 0x03, 0x04, 0x05, counter, 0x00};
    std::array<std::uint8_t, 9> covered{};
    covered[0] = static_cast<std::uint8_t>(id & 0xFF);
    covered[1] = static_cast<std::uint8_t>(id >> 8);
    for (std::size_t i = 0; i < kCrcByte; ++i) {
        covered[i + 2] = payload[i];
    }
    payload[kCrcByte] = crc8SaeJ1850(covered.data(), covered.size());
    return payload;
}

}  // namespace

// SW_E2E_008
TEST(E2eReceiverTest, AcceptsTheFirstValidMessageWhateverItsCounter) {
    CE2eReceiver receiver(kId);
    EXPECT_EQ(receiver.check(frame(9)), EE2eResult::kOk);
}

// SW_E2E_003, SW_E2E_007
TEST(E2eReceiverTest, AcceptsTheNextCounter) {
    CE2eReceiver receiver(kId);
    ASSERT_EQ(receiver.check(frame(3)), EE2eResult::kOk);
    EXPECT_EQ(receiver.check(frame(4)), EE2eResult::kOk);
    EXPECT_EQ(receiver.lostMessages(), 0u);
}

// SW_E2E_003
TEST(E2eReceiverTest, AcceptsTheWrapFromFifteenToZero) {
    CE2eReceiver receiver(kId);
    ASSERT_EQ(receiver.check(frame(15)), EE2eResult::kOk);
    EXPECT_EQ(receiver.check(frame(0)), EE2eResult::kOk);
}

// SW_E2E_006
TEST(E2eReceiverTest, ReportsARepeatedCounter) {
    CE2eReceiver receiver(kId);
    ASSERT_EQ(receiver.check(frame(5)), EE2eResult::kOk);
    EXPECT_EQ(receiver.check(frame(5)), EE2eResult::kRepeated);
}

// SW_E2E_007
TEST(E2eReceiverTest, ReportsLostMessages) {
    CE2eReceiver receiver(kId);
    ASSERT_EQ(receiver.check(frame(2)), EE2eResult::kOk);
    EXPECT_EQ(receiver.check(frame(5)), EE2eResult::kLost);
    EXPECT_EQ(receiver.lostMessages(), 2u);
}

// SW_E2E_007: the gap is counted modulo 16
TEST(E2eReceiverTest, CountsLostMessagesAcrossTheWrap) {
    CE2eReceiver receiver(kId);
    ASSERT_EQ(receiver.check(frame(14)), EE2eResult::kOk);
    EXPECT_EQ(receiver.check(frame(1)), EE2eResult::kLost);
    EXPECT_EQ(receiver.lostMessages(), 2u);
}

// SW_E2E_007: the count refers to the last check only
TEST(E2eReceiverTest, LostCountResetsOnTheNextNormalMessage) {
    CE2eReceiver receiver(kId);
    ASSERT_EQ(receiver.check(frame(2)), EE2eResult::kOk);
    ASSERT_EQ(receiver.check(frame(6)), EE2eResult::kLost);
    EXPECT_EQ(receiver.check(frame(7)), EE2eResult::kOk);
    EXPECT_EQ(receiver.lostMessages(), 0u);
}

// SW_E2E_005
TEST(E2eReceiverTest, ReportsACorruptedPayload) {
    CE2eReceiver receiver(kId);
    TPayload payload = frame(1);
    payload[0] ^= 0x01;
    EXPECT_EQ(receiver.check(payload), EE2eResult::kCorrupted);
}

// SW_E2E_005: a valid frame of another message ID fails the CRC
TEST(E2eReceiverTest, ReportsAMessageWithTheWrongId) {
    CE2eReceiver receiver(kId);
    EXPECT_EQ(receiver.check(frame(1, 0x2A, 0x100)), EE2eResult::kCorrupted);
}

// SW_E2E_009
TEST(E2eReceiverTest, CorruptedMessageDoesNotMoveTheCounter) {
    CE2eReceiver receiver(kId);
    ASSERT_EQ(receiver.check(frame(3)), EE2eResult::kOk);
    TPayload corrupted = frame(4);
    corrupted[1] ^= 0x80;
    ASSERT_EQ(receiver.check(corrupted), EE2eResult::kCorrupted);
    EXPECT_EQ(receiver.check(frame(4)), EE2eResult::kOk);
}

// SW_E2E_009
TEST(E2eReceiverTest, RepeatedMessageDoesNotMoveTheCounter) {
    CE2eReceiver receiver(kId);
    ASSERT_EQ(receiver.check(frame(3)), EE2eResult::kOk);
    ASSERT_EQ(receiver.check(frame(3)), EE2eResult::kRepeated);
    EXPECT_EQ(receiver.check(frame(4)), EE2eResult::kOk);
}

// SW_E2E_005, SW_E2E_008: a corrupted first message does not count as the first valid one
TEST(E2eReceiverTest, CorruptedFirstMessageIsNotAccepted) {
    CE2eReceiver receiver(kId);
    TPayload corrupted = frame(7);
    corrupted[2] ^= 0x10;
    ASSERT_EQ(receiver.check(corrupted), EE2eResult::kCorrupted);
    EXPECT_EQ(receiver.check(frame(12)), EE2eResult::kOk);
}

// SW_E2E_010
TEST(E2eReceiverTest, DoesNotAllocate) {
    CE2eReceiver receiver(kId);
    const auto before = cvc::test::allocationCount();
    for (std::uint8_t counter = 0; counter < 64; ++counter) {
        static_cast<void>(receiver.check(frame(static_cast<std::uint8_t>(counter % 16))));
    }
    EXPECT_EQ(cvc::test::allocationCount() - before, 0u);
}
