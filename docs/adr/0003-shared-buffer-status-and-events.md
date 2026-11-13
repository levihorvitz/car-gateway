# 0003. Shared buffer with status and event entries

- **Status:** Proposed
- **Date:** 2026-11-11

## Context

Managers depend on each other's state: the Lighting manager needs the brake status, the vehicle manager needs every unit's status. Some information is a value that changes over time, such as speed or temperature, where only the latest value matters. Other information is an occurrence that must not be missed or must take effect exactly once, such as a confirmed fault code or a "lock all doors" command.

## Decision

Managers exchange data through a shared buffer with two kinds of entries:

- **Status:** the latest value. Each write overwrites the previous one; reading consumes nothing; any number of managers can read it.
- **Event / command:** a ring buffer with a read position per reader, sequence numbers and acknowledgement. Every reader sees every entry in order; a reader that falls behind is detected by its sequence number.

Each signal is classified as status or event, and the reason is documented next to it.

## Alternatives considered

- **Managers call each other directly.** Creates a web of dependencies and hidden ordering between managers.
- **A central coordinator decides every cross-unit reaction.** One place to look, but it grows into a large class that knows every unit.
- **Only status entries.** Simpler, but a short event, such as a fault that appears and clears within one cycle, could be missed.
- **Only queues.** Every value would have to be consumed by every reader, and a lost update would not be corrected by the next one.

## Consequences

- Managers depend on the buffer, not on each other.
- Status is robust: a lost update is corrected by the next one, and a change is detected by comparing with the previous cycle.
- The buffer is used only inside the single logic thread, so it needs no locks.
- Ring buffers have a fixed size, chosen at initialisation.
