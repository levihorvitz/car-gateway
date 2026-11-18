#pragma once

#include "core/clock.hpp"

namespace cvc::core {

class CSystemClock final : public IClock {
public:
    [[nodiscard]] TTimePoint now() const override;
    void sleepUntil(TTimePoint deadline) override;
};

}  // namespace cvc::core
