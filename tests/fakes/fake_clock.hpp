#pragma once

#include "core/clock.hpp"

namespace gateway::test {

class FakeClock final : public core::IClock {
public:
    [[nodiscard]] core::TimePoint now() const override { return now_; }

    void sleepUntil(core::TimePoint deadline) override {
        if (deadline > now_) {
            now_ = deadline;
        }
    }

    void advance(core::Duration d) { now_ += d; }

private:
    core::TimePoint now_{};
};

}  // namespace gateway::test
