#include "comm/e2e_sender.hpp"

namespace gateway::comm {

CE2eSender::CE2eSender(TMessageId id) noexcept : id_(id) {}

void CE2eSender::protect(TPayload& payload) noexcept {
    static_cast<void>(payload);
}

}  // namespace gateway::comm
