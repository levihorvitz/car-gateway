#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "app/system_config.hpp"
#include "core/scheduler.hpp"
#include "core/system_clock.hpp"

using namespace std::chrono_literals;
using gateway::core::CCyclicScheduler;
using gateway::core::CSystemClock;
using gateway::core::IClock;
using gateway::core::ITask;
using gateway::core::TDuration;

namespace {

constexpr std::uint64_t kCyclesPerSecond = std::chrono::seconds(1) / gateway::app::kCyclePeriod;
constexpr long kDefaultSeconds = 10;
constexpr TDuration kSimulatedWork = 2ms;

CCyclicScheduler* gScheduler = nullptr;

extern "C" void onInterrupt(int) {
    if (gScheduler != nullptr) {
        gScheduler->requestStop();
    }
}

class CSimulatedWork final : public ITask {
public:
    CSimulatedWork(const IClock& clock, TDuration work) : clock_(clock), work_(work) {}

    void step() override {
        const auto end = clock_.now() + work_;
        while (clock_.now() < end) {
        }
    }

private:
    const IClock& clock_;
    TDuration work_;
};

class CStatsPrinter final : public ITask {
public:
    explicit CStatsPrinter(const CCyclicScheduler& scheduler) : scheduler_(scheduler) {}

    void step() override {
        if (++cycles_ % kCyclesPerSecond != 0) {
            return;
        }
        const auto stats = scheduler_.stats();
        std::printf("cycle %8llu | overruns %4llu | max start delay %7.3f ms | max execution %7.3f ms\n",
                    static_cast<unsigned long long>(cycles_),
                    static_cast<unsigned long long>(stats.overruns),
                    toMilliseconds(stats.maxStartDelay),
                    toMilliseconds(stats.maxExecutionTime));
        std::fflush(stdout);
    }

private:
    static double toMilliseconds(TDuration duration) {
        return std::chrono::duration<double, std::milli>(duration).count();
    }

    const CCyclicScheduler& scheduler_;
    std::uint64_t cycles_ = 0;
};

class CStopAfter final : public ITask {
public:
    CStopAfter(CCyclicScheduler& scheduler, std::uint64_t cycles) : scheduler_(scheduler), remaining_(cycles) {}

    void step() override {
        if (remaining_ > 0 && --remaining_ == 0) {
            scheduler_.requestStop();
        }
    }

private:
    CCyclicScheduler& scheduler_;
    std::uint64_t remaining_;
};

long parseSeconds(int argc, char** argv) {
    if (argc < 2) {
        return kDefaultSeconds;
    }
    const long seconds = std::strtol(argv[1], nullptr, 10);
    return seconds > 0 ? seconds : kDefaultSeconds;
}

}  // namespace

int main(int argc, char** argv) {
    const long seconds = parseSeconds(argc, argv);

    CSystemClock clock;
    CCyclicScheduler scheduler(clock, gateway::app::kCyclePeriod);
    CSimulatedWork work(clock, kSimulatedWork);
    CStatsPrinter printer(scheduler);
    CStopAfter stop(scheduler, static_cast<std::uint64_t>(seconds) * kCyclesPerSecond);

    if (!scheduler.registerTask(work) || !scheduler.registerTask(printer) || !scheduler.registerTask(stop)) {
        std::fprintf(stderr, "failed to register tasks\n");
        return EXIT_FAILURE;
    }

    gScheduler = &scheduler;
    std::signal(SIGINT, onInterrupt);

    std::printf("Running for %ld s at %llu Hz with %.1f ms of simulated work per cycle. Ctrl+C stops early.\n",
                seconds, static_cast<unsigned long long>(kCyclesPerSecond),
                std::chrono::duration<double, std::milli>(kSimulatedWork).count());
    scheduler.run();

    const auto stats = scheduler.stats();
    std::printf("Done: %llu cycles, %llu overruns.\n",
                static_cast<unsigned long long>(stats.cycles),
                static_cast<unsigned long long>(stats.overruns));
    return EXIT_SUCCESS;
}
