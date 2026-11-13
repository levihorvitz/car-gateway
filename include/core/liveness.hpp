#pragma once

namespace gateway::core {

// Receives a "still alive" signal from the scheduler; implemented by the watchdog feeder.
class ILivenessListener {
public:
    virtual ~ILivenessListener() = default;

    // Called at the end of every completed scheduler cycle.
    virtual void onAlive() = 0;
};

}  // namespace gateway::core
