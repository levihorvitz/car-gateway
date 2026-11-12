#pragma once

#include "core/clock.hpp"

namespace gateway::core {

class SystemClock final : public IClock {
public:
    [[nodiscard]] TimePoint now() const override;
    void sleepUntil(TimePoint deadline) override;
};

}  // namespace gateway::core
