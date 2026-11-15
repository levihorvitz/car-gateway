# 0009. Manager initialization with init(), power-on BIT inside the cycle

- **Status:** Accepted
- **Date:** 2026-11-15

## Context

Before the first cycle, every manager must prepare its state: initial values, its fault table, its entries in the shared buffer. Every unit must also pass a power-on self-test before normal operation (`SYS_BOOT_001`). Memory is never allocated after initialisation (ADR 0005), so a manager cannot be destroyed and created again to reset it.

## Decision

**Initialization.** The manager base class declares `[[nodiscard]] bool init()`:

- `main` constructs every manager first, then calls `init()` on each of them, in execution order, before `CCyclicScheduler::run()`.
- If any `init()` returns `false`, the system does not start, and the failing manager is logged.
- Calling `step()` on a manager whose `init()` has not succeeded is a programming error, caught by an `assert`.
- `init()` may be called again on a stopped system to bring a manager back to its initial state.

Constructors only receive and store dependencies (clock, shared buffer, bus); they do no work that can fail.

**Power-on BIT.** It does not run in `init()`. The vehicle manager starts in the `Starting` state; while in it, every manager runs its power-on self-test inside its normal `step()`, over as many cycles as it needs, and reports the result. The vehicle manager moves to `Normal` when every manager has passed.

`ITask` does not change: the scheduler stays unaware of initialization.

## Alternatives considered

- **Initialization in the constructor only.** No half-initialised objects, but a constructor cannot report failure without exceptions, and a manager could not be reset without being destroyed.
- **`init()` in `ITask`, called by the scheduler.** Makes every task, including simple ones, implement it, and puts start-up policy into a generic component.
- **Power-on BIT inside `init()`.** The test sends messages and waits for answers; doing it in `init()` would block start-up and need its own timeouts outside the cycle.

## Consequences

- Every manager has two phases: constructed, then initialised. The base class tracks which phase it is in.
- Start-up order is explicit in `main`.
- `Starting` is a real vehicle state (`VEHICLE_STATE` in the ICD), visible on the dashboard during the power-on self-test.
