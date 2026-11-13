#pragma once

namespace gateway::core {

// A unit of work run by the scheduler; implemented by every manager.
class ITask {
public:
    virtual ~ITask() = default;

    // Does one cycle's worth of work; called once per scheduler cycle.
    virtual void step() = 0;
};

}  // namespace gateway::core
