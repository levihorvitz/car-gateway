#include "comm/e2e_receiver.hpp"

namespace cvc::comm
{

    CE2eReceiver::CE2eReceiver(TMessageId id) noexcept : id_(id) {}

    EE2eResult CE2eReceiver::check(const TPayload &payload) noexcept
    {
        lostMessages_ = 0;
        uint8_t crc = e2eCrc(id_, payload);
        if (payload[kCrcByte] != crc)
            return EE2eResult::kCorrupted;
        uint8_t newCounter = static_cast<uint8_t>(payload[kCounterByte] & kCounterMask);

        if (!hasLastCounter_)
        {
            hasLastCounter_ = true;
            lastCounter_ = newCounter;
            return EE2eResult::kOk;
        }

        uint8_t diff = static_cast<uint8_t>((newCounter + kCounterModulo - lastCounter_) % kCounterModulo);
        lastCounter_ = newCounter;
        if (diff == 0)
            return EE2eResult::kRepeated;
        if (diff > 1)
        {
            lostMessages_ = diff - 1;
            return EE2eResult::kLost;
        }
        return EE2eResult::kOk;
    }

    std::uint8_t CE2eReceiver::lostMessages() const noexcept
    {
        return lostMessages_;
    }

} // namespace cvc::comm
