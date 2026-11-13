# Detailed design — Cyclic scheduler

Requirements: `docs/requirements/sw_scheduler.sdoc`. Architecture: [ADR 0001](../adr/0001-cyclic-executive-single-logic-thread.md).

## Files

| File | Contents |
| --- | --- |
| `include/core/clock.hpp` | `IClock` (time source interface) |
| `include/core/system_clock.hpp` | `CSystemClock`, the real clock |
| `include/core/task.hpp` | `ITask`, implemented by every manager |
| `include/core/liveness.hpp` | `ILivenessListener`, implemented by the watchdog feeder |
| `include/core/scheduler.hpp` | `CCyclicScheduler` and `SSchedulerStats` |
| `tests/fakes/fake_clock.hpp` | `CFakeClock`, for tests only |

All production code is in the namespace `gateway::core`; test helpers are in `gateway::test`.

## Usage

```cpp
gateway::core::CSystemClock clock;
gateway::core::CCyclicScheduler scheduler(clock, std::chrono::milliseconds(20), &watchdogFeeder);

if (!scheduler.registerTask(engineManager)) { /* table full: fail at start-up */ }
if (!scheduler.registerTask(brakesManager)) { /* ... */ }

scheduler.run();   // blocks until requestStop()
```

In a test, a `CFakeClock` replaces the system clock, and a task can call `requestStop()` after a set number of cycles, so `run()` returns without any test-only API.

## One cycle

`run()` plans every start at an absolute time: cycle *N* starts at `start + N × period` (`SW_SCHED_004`). For each cycle:

1. Sleep until the planned start.
2. Record the start delay (actual start − planned start) and update `maxStartDelay`.
3. Call `step()` on every task, in registration order (`SW_SCHED_002`).
4. Record the execution time and update `maxExecutionTime`.
5. Increment `cycles` and call `liveness->onAlive()` if a listener was given (`SW_SCHED_011`).
6. If a stop was requested, return (`SW_SCHED_010`).
7. If the time is already past the next planned start, count an overrun (`SW_SCHED_005`) and move the plan forward to the first planned start that has not yet passed, without running the missed cycles (`SW_SCHED_006`).

## Memory and threads

- Tasks are kept as pointers in a `std::array` of `kMaxTasks` (16); nothing is allocated after `run()` starts (`SW_SCHED_008`, `SW_SCHED_009`, ADR 0005).
- The scheduler does not own its tasks or its clock; their owners must outlive it.
- `requestStop()` sets a `std::atomic<bool>`, so it is safe from any thread.
- `stats()` reads plain fields written by the scheduler thread; call it from that thread (for example from the dashboard manager's `step()`) or after `run()` has returned.

## Requirement coverage

| Requirement | Where |
| --- | --- |
| `SW_SCHED_001` Cycle period | constructor `period`, `run()` |
| `SW_SCHED_002` Execution order | `registerTask()`, `run()` step 3 |
| `SW_SCHED_003` Start accuracy | measured in `maxStartDelay`; verified on the Raspberry Pi |
| `SW_SCHED_004` No drift | `run()` absolute planning |
| `SW_SCHED_005` Overrun detection | `run()` step 7, `stats().overruns` |
| `SW_SCHED_006` Behaviour after an overrun | `run()` step 7 |
| `SW_SCHED_007` Timing metrics | `SSchedulerStats`, `stats()` |
| `SW_SCHED_008` Task registration | `registerTask()`, `kMaxTasks` |
| `SW_SCHED_009` No dynamic allocation | `std::array` task table |
| `SW_SCHED_010` Stopping | `requestStop()`, `run()` step 6 |
| `SW_SCHED_011` Liveness signal | `ILivenessListener`, `run()` step 5 |
