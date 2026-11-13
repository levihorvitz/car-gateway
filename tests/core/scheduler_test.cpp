#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>

#include <gtest/gtest.h>

#include "core/scheduler.hpp"
#include "fakes/fake_clock.hpp"
#include "support/allocation_counter.hpp"

using namespace std::chrono_literals;
using gateway::core::CCyclicScheduler;
using gateway::core::TDuration;
using gateway::core::IClock;
using gateway::core::ILivenessListener;
using gateway::core::ITask;
using gateway::core::TTimePoint;
using gateway::test::CFakeClock;

namespace {

constexpr TDuration kPeriod = 20ms;

class CStopAfter final : public ITask {
public:
    CStopAfter(CCyclicScheduler& scheduler, std::uint64_t cycles)
        : scheduler_(scheduler), remaining_(cycles) {}

    void step() override {
        if (remaining_ > 0 && --remaining_ == 0) {
            scheduler_.requestStop();
        }
    }

private:
    CCyclicScheduler& scheduler_;
    std::uint64_t remaining_;
};

class CStartTimeRecorder final : public ITask {
public:
    explicit CStartTimeRecorder(const IClock& clock) : clock_(clock) {}

    void step() override {
        if (count_ < times_.size()) {
            times_[count_] = clock_.now();
        }
        ++count_;
    }

    [[nodiscard]] std::size_t count() const { return count_; }
    [[nodiscard]] TDuration sinceFirst(std::size_t cycle) const { return times_.at(cycle) - times_.at(0); }

private:
    const IClock& clock_;
    std::array<TTimePoint, 64> times_{};
    std::size_t count_ = 0;
};

class CBusy final : public ITask {
public:
    CBusy(CFakeClock& clock, TDuration each) : clock_(clock), each_(each) {}

    void step() override { clock_.advance(each_); }

private:
    CFakeClock& clock_;
    TDuration each_;
};

class CSlowOnce final : public ITask {
public:
    CSlowOnce(CFakeClock& clock, std::size_t cycle, TDuration duration)
        : clock_(clock), cycle_(cycle), duration_(duration) {}

    void step() override {
        if (current_ == cycle_) {
            clock_.advance(duration_);
        }
        ++current_;
    }

private:
    CFakeClock& clock_;
    std::size_t cycle_;
    TDuration duration_;
    std::size_t current_ = 0;
};

class CCallCounter final : public ITask {
public:
    void step() override { ++calls_; }

    [[nodiscard]] std::uint64_t calls() const { return calls_; }

private:
    std::uint64_t calls_ = 0;
};

class COrderLog {
public:
    void add(int id) {
        if (size_ < entries_.size()) {
            entries_[size_] = id;
        }
        ++size_;
    }

    [[nodiscard]] std::size_t size() const { return size_; }
    [[nodiscard]] int at(std::size_t index) const { return entries_.at(index); }

private:
    std::array<int, 64> entries_{};
    std::size_t size_ = 0;
};

class CTagged final : public ITask {
public:
    CTagged(COrderLog& log, int id) : log_(log), id_(id) {}

    void step() override { log_.add(id_); }

private:
    COrderLog& log_;
    int id_;
};

class CLivenessCounter final : public ILivenessListener {
public:
    void onAlive() override { ++signals_; }

    [[nodiscard]] std::uint64_t signals() const { return signals_; }

private:
    std::uint64_t signals_ = 0;
};

class CLateClock final : public IClock {
public:
    explicit CLateClock(TDuration lateness) : lateness_(lateness) {}

    [[nodiscard]] TTimePoint now() const override { return now_; }

    void sleepUntil(TTimePoint deadline) override {
        if (deadline > now_) {
            now_ = deadline;
        }
        now_ += lateness_;
    }

private:
    TTimePoint now_{};
    TDuration lateness_;
};

class CRegisterDuringRun final : public ITask {
public:
    CRegisterDuringRun(CCyclicScheduler& scheduler, ITask& extra)
        : scheduler_(scheduler), extra_(extra) {}

    void step() override {
        accepted_ = scheduler_.registerTask(extra_);
        scheduler_.requestStop();
    }

    [[nodiscard]] bool accepted() const { return accepted_; }

private:
    CCyclicScheduler& scheduler_;
    ITask& extra_;
    bool accepted_ = true;
};

class CAllocationProbe final : public ITask {
public:
    CAllocationProbe(CCyclicScheduler& scheduler, std::uint64_t cycles)
        : scheduler_(scheduler), cycles_(cycles) {}

    void step() override {
        ++current_;
        if (current_ == 1) {
            first_ = gateway::test::allocationCount();
        }
        if (current_ == cycles_) {
            last_ = gateway::test::allocationCount();
            scheduler_.requestStop();
        }
    }

    [[nodiscard]] std::size_t allocations() const { return last_ - first_; }

private:
    CCyclicScheduler& scheduler_;
    std::uint64_t cycles_;
    std::uint64_t current_ = 0;
    std::size_t first_ = 0;
    std::size_t last_ = 0;
};

class CStopDuringSleepClock final : public IClock {
public:
    explicit CStopDuringSleepClock(std::uint64_t stopOnSleep) : stopOnSleep_(stopOnSleep) {}

    void attach(CCyclicScheduler& scheduler) { scheduler_ = &scheduler; }

    [[nodiscard]] TTimePoint now() const override { return now_; }

    void sleepUntil(TTimePoint deadline) override {
        if (deadline > now_) {
            now_ = deadline;
        }
        ++sleeps_;
        if (sleeps_ == stopOnSleep_ && scheduler_ != nullptr) {
            scheduler_->requestStop();
        }
    }

private:
    TTimePoint now_{};
    std::uint64_t stopOnSleep_;
    std::uint64_t sleeps_ = 0;
    CCyclicScheduler* scheduler_ = nullptr;
};

}  // namespace

// SW_SCHED_001
TEST(CyclicSchedulerTest, StartsANewCycleEveryPeriod) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CStartTimeRecorder recorder(clock);
    CStopAfter stop(scheduler, 5);
    ASSERT_TRUE(scheduler.registerTask(recorder));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    ASSERT_EQ(recorder.count(), 5u);
    for (std::size_t cycle = 1; cycle < 5; ++cycle) {
        EXPECT_EQ(recorder.sinceFirst(cycle) - recorder.sinceFirst(cycle - 1), kPeriod);
    }
}

// SW_SCHED_002
TEST(CyclicSchedulerTest, RunsEveryTaskOncePerCycleInRegistrationOrder) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    COrderLog log;
    CTagged first(log, 1);
    CTagged second(log, 2);
    CTagged third(log, 3);
    CStopAfter stop(scheduler, 3);
    ASSERT_TRUE(scheduler.registerTask(first));
    ASSERT_TRUE(scheduler.registerTask(second));
    ASSERT_TRUE(scheduler.registerTask(third));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    const std::array<int, 9> expected{1, 2, 3, 1, 2, 3, 1, 2, 3};
    ASSERT_EQ(log.size(), expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(log.at(i), expected[i]);
    }
}

// SW_SCHED_004
TEST(CyclicSchedulerTest, DoesNotDriftWhenTasksTakeTime) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CStartTimeRecorder recorder(clock);
    CBusy busy(clock, 7ms);
    CStopAfter stop(scheduler, 10);
    ASSERT_TRUE(scheduler.registerTask(recorder));
    ASSERT_TRUE(scheduler.registerTask(busy));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    ASSERT_EQ(recorder.count(), 10u);
    for (std::size_t cycle = 0; cycle < 10; ++cycle) {
        EXPECT_EQ(recorder.sinceFirst(cycle), cycle * kPeriod);
    }
}

// SW_SCHED_004
TEST(CyclicSchedulerTest, DoesNotDriftWhenWakeUpsAreLate) {
    CLateClock clock(3ms);
    CCyclicScheduler scheduler(clock, kPeriod);
    CStartTimeRecorder recorder(clock);
    CStopAfter stop(scheduler, 10);
    ASSERT_TRUE(scheduler.registerTask(recorder));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    ASSERT_EQ(recorder.count(), 10u);
    for (std::size_t cycle = 0; cycle < 10; ++cycle) {
        EXPECT_EQ(recorder.sinceFirst(cycle), cycle * kPeriod);
    }
}

// SW_SCHED_005
TEST(CyclicSchedulerTest, CountsAnOverrunWhenTasksRunPastTheNextStart) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CSlowOnce slow(clock, 1, 25ms);
    CStopAfter stop(scheduler, 4);
    ASSERT_TRUE(scheduler.registerTask(slow));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    EXPECT_EQ(scheduler.stats().overruns, 1u);
}

// SW_SCHED_005
TEST(CyclicSchedulerTest, NoOverrunWhenTasksFinishBeforeTheNextStart) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CBusy busy(clock, 19ms);
    CStopAfter stop(scheduler, 5);
    ASSERT_TRUE(scheduler.registerTask(busy));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    EXPECT_EQ(scheduler.stats().overruns, 0u);
}

// SW_SCHED_005
TEST(CyclicSchedulerTest, FinishingExactlyAtTheNextStartIsNotAnOverrun) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CBusy busy(clock, kPeriod);
    CStopAfter stop(scheduler, 5);
    ASSERT_TRUE(scheduler.registerTask(busy));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    EXPECT_EQ(scheduler.stats().overruns, 0u);
}

// SW_SCHED_006
TEST(CyclicSchedulerTest, SkipsMissedCyclesAfterAnOverrun) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CStartTimeRecorder recorder(clock);
    CSlowOnce slow(clock, 0, 45ms);
    CStopAfter stop(scheduler, 3);
    ASSERT_TRUE(scheduler.registerTask(recorder));
    ASSERT_TRUE(scheduler.registerTask(slow));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    ASSERT_EQ(recorder.count(), 3u);
    EXPECT_EQ(recorder.sinceFirst(1), 60ms);
    EXPECT_EQ(recorder.sinceFirst(2), 80ms);
    EXPECT_EQ(scheduler.stats().cycles, 3u);
    EXPECT_EQ(scheduler.stats().overruns, 1u);
}

// SW_SCHED_007
TEST(CyclicSchedulerTest, StatsAreZeroBeforeRun) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);

    const auto stats = scheduler.stats();

    EXPECT_EQ(stats.cycles, 0u);
    EXPECT_EQ(stats.overruns, 0u);
    EXPECT_EQ(stats.maxStartDelay, 0ns);
    EXPECT_EQ(stats.maxExecutionTime, 0ns);
}

// SW_SCHED_007
TEST(CyclicSchedulerTest, ReportsCyclesAndMaximumExecutionTime) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CBusy busy(clock, 3ms);
    CSlowOnce slow(clock, 1, 12ms);
    CStopAfter stop(scheduler, 3);
    ASSERT_TRUE(scheduler.registerTask(busy));
    ASSERT_TRUE(scheduler.registerTask(slow));
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    EXPECT_EQ(scheduler.stats().cycles, 3u);
    EXPECT_EQ(scheduler.stats().maxExecutionTime, 15ms);
}

// SW_SCHED_003, SW_SCHED_007
TEST(CyclicSchedulerTest, ReportsMaximumStartDelay) {
    CLateClock clock(3ms);
    CCyclicScheduler scheduler(clock, kPeriod);
    CStopAfter stop(scheduler, 4);
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    EXPECT_EQ(scheduler.stats().maxStartDelay, 3ms);
}

// SW_SCHED_008
TEST(CyclicSchedulerTest, AcceptsUpToTheMaximumNumberOfTasks) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    std::array<CCallCounter, CCyclicScheduler::kMaxTasks + 1> tasks{};

    for (std::size_t i = 0; i < CCyclicScheduler::kMaxTasks; ++i) {
        EXPECT_TRUE(scheduler.registerTask(tasks[i]));
    }
    EXPECT_FALSE(scheduler.registerTask(tasks[CCyclicScheduler::kMaxTasks]));
}

// SW_SCHED_008
TEST(CyclicSchedulerTest, RejectsRegistrationAfterStart) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CCallCounter extra;
    CRegisterDuringRun registrar(scheduler, extra);
    ASSERT_TRUE(scheduler.registerTask(registrar));

    scheduler.run();

    EXPECT_FALSE(registrar.accepted());
    EXPECT_EQ(extra.calls(), 0u);
}

// SW_SCHED_009
TEST(CyclicSchedulerTest, DoesNotAllocateWhileRunning) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CAllocationProbe probe(scheduler, 50);
    ASSERT_TRUE(scheduler.registerTask(probe));

    scheduler.run();

    EXPECT_EQ(probe.allocations(), 0u);
}

// SW_SCHED_010
TEST(CyclicSchedulerTest, FinishesTheCurrentCycleThenStops) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CStopAfter stop(scheduler, 2);
    CCallCounter after;
    ASSERT_TRUE(scheduler.registerTask(stop));
    ASSERT_TRUE(scheduler.registerTask(after));

    scheduler.run();

    EXPECT_EQ(after.calls(), 2u);
    EXPECT_EQ(scheduler.stats().cycles, 2u);
}

// SW_SCHED_010
TEST(CyclicSchedulerTest, CanBeStoppedFromAnotherThread) {
    CFakeClock clock;
    CCyclicScheduler scheduler(clock, kPeriod);
    CCallCounter counter;
    ASSERT_TRUE(scheduler.registerTask(counter));

    std::thread stopper([&scheduler] {
        std::this_thread::sleep_for(10ms);
        scheduler.requestStop();
    });
    scheduler.run();
    stopper.join();

    EXPECT_GT(scheduler.stats().cycles, 0u);
}

// SW_SCHED_010
TEST(CyclicSchedulerTest, DoesNotStartAnotherCycleWhenStopArrivesDuringSleep) {
    CStopDuringSleepClock clock(3);
    CCyclicScheduler scheduler(clock, kPeriod);
    clock.attach(scheduler);
    CCallCounter counter;
    ASSERT_TRUE(scheduler.registerTask(counter));

    scheduler.run();

    EXPECT_EQ(counter.calls(), 2u);
    EXPECT_EQ(scheduler.stats().cycles, 2u);
}

// SW_SCHED_011
TEST(CyclicSchedulerTest, SignalsLivenessOncePerCompletedCycle) {
    CFakeClock clock;
    CLivenessCounter liveness;
    CCyclicScheduler scheduler(clock, kPeriod, &liveness);
    CStopAfter stop(scheduler, 5);
    ASSERT_TRUE(scheduler.registerTask(stop));

    scheduler.run();

    EXPECT_EQ(liveness.signals(), 5u);
}
