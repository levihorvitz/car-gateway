#pragma once

#include <cstdint>

#include "comm/e2e.hpp"

namespace cvc::comm {

class CE2eSender {
public:
    explicit CE2eSender(TMessageId id) noexcept;

    void protect(TPayload& payload) noexcept;

private:
    TMessageId id_;
    std::uint8_t counter_ = 0;
};

}  // namespace cvc::comm
