#pragma once

#include "core/clock.hpp"

namespace gateway::core {

class CSystemClock final : public IClock {
public:
    [[nodiscard]] TTimePoint now() const override;
    void sleepUntil(TTimePoint deadline) override;
};

}  // namespace gateway::core
