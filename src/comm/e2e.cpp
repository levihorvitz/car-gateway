#include "comm/e2e.hpp"

namespace gateway::comm {

std::uint8_t e2eCrc(TMessageId id, const TPayload& payload) noexcept {
    static_cast<void>(id);
    static_cast<void>(payload);
    return 0;
}

}  // namespace gateway::comm
