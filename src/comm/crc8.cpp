#include "comm/crc8.hpp"

namespace gateway::comm {

std::uint8_t crc8SaeJ1850(const std::uint8_t* data, std::size_t length) noexcept {
    static_cast<void>(data);
    static_cast<void>(length);
    return 0;
}

}  // namespace gateway::comm
