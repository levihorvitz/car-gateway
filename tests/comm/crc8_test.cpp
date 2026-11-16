#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include "comm/crc8.hpp"
#include "comm/e2e.hpp"

using gateway::comm::crc8SaeJ1850;
using gateway::comm::e2eCrc;
using gateway::comm::TPayload;

namespace {

template <std::size_t N>
std::uint8_t crcOf(const std::array<std::uint8_t, N>& data) {
    return crc8SaeJ1850(data.data(), data.size());
}

}  // namespace

// SW_E2E_001: standard check value of CRC-8 SAE J1850 over the ASCII string "123456789"
TEST(Crc8SaeJ1850Test, MatchesTheStandardCheckValue) {
    const std::array<std::uint8_t, 9> ascii{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    EXPECT_EQ(crcOf(ascii), 0x4B);
}

// SW_E2E_001: reference values published for this CRC
TEST(Crc8SaeJ1850Test, MatchesReferenceValues) {
    EXPECT_EQ(crcOf(std::array<std::uint8_t, 4>{0x00, 0x00, 0x00, 0x00}), 0x59);
    EXPECT_EQ(crcOf(std::array<std::uint8_t, 3>{0xF2, 0x01, 0x83}), 0x37);
    EXPECT_EQ(crcOf(std::array<std::uint8_t, 4>{0x0F, 0xAA, 0x00, 0x55}), 0x79);
    EXPECT_EQ(crcOf(std::array<std::uint8_t, 4>{0x00, 0xFF, 0x55, 0x11}), 0xB8);
    EXPECT_EQ(crcOf(std::array<std::uint8_t, 9>{0x33, 0x22, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}), 0xCB);
    EXPECT_EQ(crcOf(std::array<std::uint8_t, 3>{0x92, 0x6B, 0x55}), 0x8C);
    EXPECT_EQ(crcOf(std::array<std::uint8_t, 4>{0xFF, 0xFF, 0xFF, 0xFF}), 0x74);
}

// SW_E2E_001: no data leaves only the initial value and the final XOR
TEST(Crc8SaeJ1850Test, EmptyDataGivesZero) {
    EXPECT_EQ(crc8SaeJ1850(nullptr, 0), 0x00);
}

// SW_E2E_002
TEST(E2eCrcTest, CoversTheMessageIdAndBytesZeroToSix) {
    TPayload payload{0x10, 0x27, 0x5A, 0x01, 0x00, 0x00, 0x03, 0x00};
    EXPECT_EQ(e2eCrc(0x100, payload), 0x15);
    EXPECT_EQ(e2eCrc(0x110, payload), 0x0B);
}

// SW_E2E_002: byte 7 holds the CRC itself and is not covered
TEST(E2eCrcTest, IgnoresByteSeven) {
    TPayload payload{0x10, 0x27, 0x5A, 0x01, 0x00, 0x00, 0x03, 0x00};
    const auto expected = e2eCrc(0x100, payload);
    payload[7] = 0xAB;
    EXPECT_EQ(e2eCrc(0x100, payload), expected);
}
