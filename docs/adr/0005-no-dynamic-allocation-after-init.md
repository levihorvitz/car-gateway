# 0005. No dynamic memory allocation after initialisation

- **Status:** Proposed
- **Date:** 2026-11-11

## Context

The cyclic loop must start every cycle on time and finish its work within 20 ms. Dynamic memory allocation (`new`, `malloc`, and containers that grow, such as `std::vector::push_back` or `std::string` concatenation) can take an unpredictable amount of time, can fragment memory over a long run, and can fail. Embedded and safety-related software commonly forbids it after start-up for these reasons.

## Decision

All memory is allocated during initialisation. After the cyclic loop starts, no code that runs in the loop allocates memory dynamically. Buffers, queues, task tables and fault tables have fixed sizes, set at initialisation.

## Alternatives considered

- **Allow allocation everywhere.** Easier to write, but timing becomes unpredictable and long runs risk fragmentation.
- **Custom allocators or memory pools for everything.** Keeps allocation but makes it predictable; more complex than needed when sizes are known in advance.

## Consequences

- Sizes must be chosen up front, for example a maximum of 16 tasks (`SW_SCHED_008`).
- Prefer `std::array`, fixed-capacity containers and pre-sized buffers; avoid growing containers, `std::string` building and `std::function` with captures in the loop.
- Allocation during initialisation, and in code outside the loop such as the dashboard server, is allowed.
- Tests check the rule by counting allocations while the loop runs.
