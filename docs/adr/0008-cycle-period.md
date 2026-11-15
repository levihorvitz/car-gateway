# 0008. Cycle period of 20 ms

- **Status:** Proposed
- **Date:** 2026-11-15

## Context

The cycle period of the scheduler (ADR 0001) is the base of every timing figure in the system: reaction times, fault confirmation, silent-unit detection, and the send rate of every message in the ICD. It was first chosen during design; this ADR records why it is right, deriving it from the system requirements and checking it against bus load and CPU load. Verification method: Analysis.

## Decision

The cycle period is **20 ms (50 Hz)**. It is defined once, as `kCyclePeriod` in `include/app/system_config.hpp` (`SW_SCHED_001`), and every message in `icd/car_gateway.dbc` is sent at the same period.

## Analysis

**1. Reaction budget.** The tightest requirements are reactions within 40 ms: brake lights on and off (`SYS_LIGHT_001`, `SYS_LIGHT_002`) and entering Limited or Safe after a confirmed fault (`SYS_STATE_002`, `SYS_STATE_003`). In the worst case a message arrives just after a cycle has read its queues, so it is seen one cycle later; reporting managers run before reacting managers (ADR 0002), so the reaction is decided and sent in that same cycle. The worst case is therefore two cycles, which fits 40 ms only if the period is at most **20 ms**.

**2. Other timing requirements.** At 20 ms, a unit is declared silent after five missed status messages (100 ms, `SYS_MON_002`), and a fault condition is sampled three times before it is confirmed (60 ms, `SYS_FAULT_002`).

**3. CAN bus load.** An 8-byte standard frame is at most about 135 bits on the wire, including worst-case bit stuffing. The full system has about ten periodic messages:

| Case | Frames per second | Load at 500 kbit/s |
| --- | --- | --- |
| Phase 1 (5 messages) | 250 | 34 kbit/s, about 7% |
| Full system (about 10 messages) | 500 | 68 kbit/s, about 14% |

Both leave a large margin for diagnostic traffic and later growth.

**4. CPU load.** The scheduler demo ran 250 cycles in 5 s with 2 ms of simulated work per cycle: no overruns, maximum execution time 2.75 ms, on WSL without real-time priority. The managers do little work per cycle, so the target is to stay below half the period (10 ms) of execution per cycle on the Raspberry Pi, leaving room for jitter. This is measured on the target in phase 3.

**5. The physical world.** At 100 km/h a car moves about 0.56 m in 20 ms: changes in the vehicle are seen well before they matter.

## Alternatives considered

- **10 ms (100 Hz).** Halves the worst-case reaction to 20 ms, but doubles bus load and CPU load, and halves the margin for jitter, while no requirement needs it.
- **25 ms or longer.** The worst-case reaction becomes 50 ms or more and breaks the 40 ms requirements.
- **No fixed period (event-driven).** Rejected in ADR 0001.

## Consequences

- Every unit sends its status every 20 ms, and the ICD sets `GenMsgCycleTime` to 20 for every message.
- Each cycle has an execution budget: tasks must finish well within 20 ms, and overruns are counted and reported (`SW_SCHED_005`).
- Changing the period is a change request: it touches `kCyclePeriod`, the timing requirements, the ICD and this ADR together.
