#pragma once

#include "core/clock.hpp"

namespace gateway::test {

class CFakeClock final : public core::IClock {
public:
    [[nodiscard]] core::TTimePoint now() const override { return now_; }

    void sleepUntil(core::TTimePoint deadline) override {
        if (deadline > now_) {
            now_ = deadline;
        }
    }

    void advance(core::TDuration d) { now_ += d; }

private:
    core::TTimePoint now_{};
};

}  // namespace gateway::test
