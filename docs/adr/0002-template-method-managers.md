# 0002. Managers share one base class (Template Method)

- **Status:** Accepted
- **Date:** 2026-11-11

## Context

The central computer has one manager per vehicle unit, plus a vehicle manager. Every manager must do the same things each cycle: read its messages, check E2E protection, check that its unit is still reporting, detect status changes, run the fault mechanism, and publish its status. Only a few parts differ per unit: how its messages are decoded, its valid ranges, how it reacts to other units, and what its BIT checks.

## Decision

All managers derive from one abstract base class. The base class implements `step()` once, as a fixed sequence of steps, and calls small virtual hooks for the parts that differ. Each derived manager implements only those hooks.

## Alternatives considered

- **Each manager implements everything itself.** Simple at first, but the shared logic (liveness, debounce, latching) would be copied many times and would drift apart as bugs are fixed in one copy and not the others.
- **Composition (a manager holds a set of reusable components).** More flexible, but more objects and wiring for a sequence of steps that is the same for every manager.
- **A data-driven manager configured from tables.** Avoids classes per unit, but unit-specific reactions are real logic and are clearer as code.

## Consequences

- Shared logic is written and tested once, against a simple fake manager.
- Adding a unit means writing one small derived class.
- The base class's sequence of steps is a contract for every manager; changing it affects all of them and needs the approval of both developers.
- The inheritance hierarchy stays one level deep.
