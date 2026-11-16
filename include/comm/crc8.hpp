#pragma once

#include <cstddef>
#include <cstdint>

namespace gateway::comm {

[[nodiscard]] std::uint8_t crc8SaeJ1850(const std::uint8_t* data, std::size_t length) noexcept;

}  // namespace gateway::comm
