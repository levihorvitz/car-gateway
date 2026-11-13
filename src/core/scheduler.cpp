#include "core/scheduler.hpp"

namespace gateway::core {

CCyclicScheduler::CCyclicScheduler(IClock& clock, TDuration period, ILivenessListener* liveness)
    : clock_(clock), period_(period), liveness_(liveness) {}

bool CCyclicScheduler::registerTask(ITask& task) {
    if (started_ || taskCount_ >= kMaxTasks) {
        return false;
    }
    tasks_[taskCount_] = &task;
    ++taskCount_;
    return true;
}

void CCyclicScheduler::run() {
    started_ = true;

    // Every start is planned at an absolute time: start + N x period (SW_SCHED_004).
    TTimePoint plannedStart = clock_.now();

    while (true) {
        // 1. Sleep until the planned start.
        clock_.sleepUntil(plannedStart);

        // 2. Record the start delay.
        const TTimePoint actualStart = clock_.now();
        const TDuration startDelay = actualStart - plannedStart;
        if (startDelay > stats_.maxStartDelay) {
            stats_.maxStartDelay = startDelay;
        }

        // 3. Run every task in registration order (SW_SCHED_002).
        for (std::size_t i = 0; i < taskCount_; ++i) {
            tasks_[i]->step();
        }

        // 4. Record the execution time.
        const TTimePoint finished = clock_.now();
        const TDuration executionTime = finished - actualStart;
        if (executionTime > stats_.maxExecutionTime) {
            stats_.maxExecutionTime = executionTime;
        }

        // 5. Count the cycle and signal liveness (SW_SCHED_011).
        ++stats_.cycles;
        if (liveness_ != nullptr) {
            liveness_->onAlive();
        }

        // 6. Stop after the current cycle if requested (SW_SCHED_010).
        if (stopRequested_.load()) {
            return;
        }

        // 7. On overrun, skip to the first planned start that has not yet passed
        //    (SW_SCHED_005, SW_SCHED_006). Finishing exactly on time is not an overrun.
        plannedStart += period_;
        if (finished > plannedStart) {
            ++stats_.overruns;
            const TDuration behind = finished - plannedStart;
            const auto missed = (behind + period_ - TDuration{1}) / period_;  // ceil(behind / period)
            plannedStart += missed * period_;
        }
    }
}

void CCyclicScheduler::requestStop() noexcept {
    stopRequested_.store(true);
}

SSchedulerStats CCyclicScheduler::stats() const noexcept {
    return stats_;
}

}  // namespace gateway::core
