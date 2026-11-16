#pragma once

#include <cstdint>

#include "comm/e2e.hpp"

namespace gateway::comm {

class CE2eReceiver {
public:
    explicit CE2eReceiver(TMessageId id) noexcept;

    [[nodiscard]] EE2eResult check(const TPayload& payload) noexcept;
    [[nodiscard]] std::uint8_t lostMessages() const noexcept;

private:
    TMessageId id_;
    std::uint8_t lastCounter_ = 0;
    bool hasLastCounter_ = false;
    std::uint8_t lostMessages_ = 0;
};

}  // namespace gateway::comm
