#include "comm/e2e_receiver.hpp"

namespace gateway::comm {

CE2eReceiver::CE2eReceiver(TMessageId id) noexcept : id_(id) {}

EE2eResult CE2eReceiver::check(const TPayload& payload) noexcept {
    static_cast<void>(payload);
    return EE2eResult::kOk;
}

std::uint8_t CE2eReceiver::lostMessages() const noexcept {
    return lostMessages_;
}

}  // namespace gateway::comm
