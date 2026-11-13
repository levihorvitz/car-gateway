# 0001. Cyclic executive at 50 Hz with one logic thread

- **Status:** Proposed
- **Date:** 2026-11-11

## Context

The central computer must react to unit status within 40 ms (`SYS_LIGHT_001`, `SYS_STATE_002`, `SYS_STATE_003`) and confirm faults after 60 ms (`SYS_FAULT_002`). Its logic must behave the same way every time so that faults can be reproduced and tests are reliable. At the same time, reading from the bus involves waiting on hardware.

## Decision

All logic runs in a cyclic executive: one thread that starts a cycle every 20 ms at an absolute planned time and calls every task once, in a fixed order. Dedicated I/O threads read the bus and push frames into queues; the logic thread reads the queues without blocking.

## Alternatives considered

- **One thread per task.** Tasks could block on I/O independently, but every shared value would need locking, the order of execution would change from run to run, and timing bugs would be hard to reproduce.
- **Event-driven logic (react to each frame as it arrives).** Lower latency for a single frame, but behaviour depends on arrival order, load is unbounded under a burst of frames, and liveness checks still need a timer.
- **Everything in one thread, including I/O.** Simplest, but a blocking read would stall the whole cycle.

## Consequences

- Logic is deterministic and needs no locks inside the logic thread.
- Tests can drive the loop with a fake clock and reproduce any timing exactly.
- Every task must be short and must never block; a task that runs too long causes an overrun, which the scheduler detects and reports.
- The queues between the I/O threads and the logic thread are the only point where threads meet, and must be designed and tested carefully.
