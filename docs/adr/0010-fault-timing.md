# 0010. Fault timing measured in elapsed clock time

- **Status:** Accepted
- **Date:** 2026-11-15

## Context

Several requirements are durations: a fault is confirmed after its condition persists for 60 ms (`SYS_FAULT_002`), cleared after it is absent for 1 s (`SYS_FAULT_003`), and a unit is declared silent 100 ms after its last valid message (`SYS_MON_002`). Counting cycles is the obvious way to measure them, but after an overrun the scheduler skips cycles (`SW_SCHED_006`): three counted cycles may then take 80 ms or more, and the meaning of a count changes whenever the cycle period changes.

## Decision

Every timed condition is measured in elapsed time from the injected `IClock`, never by counting cycles:

- When a fault condition is first seen, the manager stores the time from `clock.now()`. The fault is confirmed when the condition is still present and `now - firstSeen >= kFaultConfirmTime`.
- A confirmed non-critical fault clears when the condition has been absent and `now - lastSeen >= kFaultClearTime`.
- A unit is declared silent when `now - lastValidMessage >= kUnitSilenceTime`.
- The durations are named constants of type `TDuration` in `include/app/system_config.hpp`, next to `kCyclePeriod`, each with the requirement it implements.

Managers receive `IClock&` in their constructor, like the scheduler.

## Alternatives considered

- **Counting cycles.** Simple, but wrong after skipped cycles, and every count must change when the period changes.
- **Passing the cycle start time to `step()`.** Every manager would see exactly the same time in a cycle, but `ITask` would change for a difference of microseconds. Can be revisited with a new ADR if it ever matters.

## Consequences

- Timing stays correct after overruns and if the cycle period changes.
- Unit tests drive fault timing with the fake clock, to the exact millisecond.
- Times are checked once per cycle, so a duration is detected up to one period late: a 60 ms confirmation happens between 60 and 80 ms. The requirements say "at least", so this is within them.
