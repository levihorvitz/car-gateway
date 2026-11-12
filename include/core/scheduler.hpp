#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "core/clock.hpp"
#include "core/liveness.hpp"
#include "core/task.hpp"

namespace gateway::core {

// Timing metrics collected by the scheduler while it runs.
struct SchedulerStats {
    std::uint64_t cycles = 0;         // Number of cycles completed.
    std::uint64_t overruns = 0;       // Cycles that ran past the next planned start.
    Duration maxStartDelay{0};        // Largest gap between planned and actual cycle start.
    Duration maxExecutionTime{0};     // Longest time taken to run all tasks in one cycle.
};

// Runs registered tasks in order, once per fixed period, on a single thread.
class CyclicScheduler {
public:
    // Maximum number of tasks; the task table has a fixed size (no dynamic allocation).
    static constexpr std::size_t kMaxTasks = 16;

    // clock: time source; period: cycle length; liveness: optional, notified after each cycle.
    CyclicScheduler(IClock& clock, Duration period, ILivenessListener* liveness = nullptr);

    // Not copyable or movable: holds references to external objects and an atomic flag.
    CyclicScheduler(const CyclicScheduler&) = delete;
    CyclicScheduler& operator=(const CyclicScheduler&) = delete;
    CyclicScheduler(CyclicScheduler&&) = delete;
    CyclicScheduler& operator=(CyclicScheduler&&) = delete;
    ~CyclicScheduler() = default;

    // Adds a task to the end of the run order; returns false if it cannot be added.
    [[nodiscard]] bool registerTask(ITask& task);
    // Runs the cycle loop; blocks until requestStop() is called.
    void run();
    // Asks run() to return after the current cycle; safe to call from any thread.
    void requestStop() noexcept;
    // Returns a copy of the metrics; call from the scheduler thread or after run() returns.
    [[nodiscard]] SchedulerStats stats() const noexcept;

private:
    IClock& clock_;                       // Time source (not owned).
    const Duration period_;               // Length of one cycle.
    ILivenessListener* const liveness_;   // Optional liveness listener (not owned, may be null).

    std::array<ITask*, kMaxTasks> tasks_{};  // Registered tasks, in run order (not owned).
    std::size_t taskCount_ = 0;              // Number of entries in tasks_ that are in use.

    bool started_ = false;                     // True once run() has started.
    std::atomic<bool> stopRequested_{false};   // Set by requestStop(), checked by run().

    SchedulerStats stats_{};  // Metrics updated by run().
};

}  // namespace gateway::core
