#include "comm/crc8.hpp"

namespace gateway::comm
{

    std::uint8_t crc8SaeJ1850(const std::uint8_t *data, std::size_t length) noexcept
    {
        uint8_t crc = 0xFF;
        for (int i = 0; i < length; i++)
        {
            crc ^= data[i];
            for (int j = 0; j < 8; j++)
            {
                if (crc & 0x80)
                {
                    crc *= 2;
                    crc ^= 0x1D;
                }
                else
                    crc *= 2;
            }
        }
        return crc ^ 0xFF;
    }

} // namespace gateway::comm
