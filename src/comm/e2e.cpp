#include "comm/e2e.hpp"
#include "comm/crc8.hpp"

namespace gateway::comm
{

    std::uint8_t e2eCrc(TMessageId id, const TPayload &payload) noexcept
    {
        std::uint8_t data[9] = {0};
        data[0] = id & 0xFF;
        data[1] = id >> 8;
        for (size_t i = 0; i <kCrcByte; i++)
            data[i+2] = payload[i];
        return crc8SaeJ1850(data, 9);
    }

} // namespace gateway::comm
