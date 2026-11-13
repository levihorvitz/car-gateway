# Coding guidelines

Rules for all C++ code in this repository. A change to these rules goes through a pull request, like any other change.

## Naming

Every type name starts with a letter that says what kind of type it is.

| Kind | Prefix | Example |
| --- | --- | --- |
| Interface (abstract class, pure virtual functions only) | `I` | `IClock`, `ITask` |
| Class | `C` | `CCyclicScheduler`, `CSystemClock` |
| Struct (plain data, no behaviour) | `S` | `SSchedulerStats` |
| Enum (always `enum class`) | `E` | `EVehicleState` |
| Type alias (`using`) | `T` | `TDuration`, `TTimePoint` |

| Other names | Style | Example |
| --- | --- | --- |
| Functions | camelCase | `registerTask()`, `requestStop()` |
| Variables and parameters | camelCase | `taskCount`, `deadline` |
| Private member variables | camelCase with a trailing `_` | `taskCount_`, `clock_` |
| Constants and enum values | `k` + PascalCase | `kMaxTasks`, `kLimited` |
| Namespaces | lower case | `gateway::core` |
| Files | snake_case | `system_clock.hpp`, `scheduler_test.cpp` |

## Files and folders

- Headers use `.hpp`, sources use `.cpp`.
- Public headers live in `include/<area>/`, their sources in `src/<area>/`, with the same file name.
- Each interface has its own header, separate from every implementation.
- One class per header and source pair.
- Every header starts with `#pragma once` and includes everything it uses.
- The namespace matches the folder: `include/core/` is `gateway::core`.
- Test-only code lives under `tests/`, in the namespace `gateway::test`.
- Never write `using namespace` in a header.

## Language rules

- C++17 (ADR 0006), no compiler extensions.
- No dynamic memory allocation after initialisation (ADR 0005).
- Functions whose result must be checked are `[[nodiscard]]`.
- Time is always a `std::chrono` type, never a plain integer.
- Classes that hold references or run-time state delete their copy and move operations.
- The code builds without warnings; CI treats warnings as errors.

## Traceability

- Every test names the requirement it verifies, for example `// SW_SCHED_006`.
