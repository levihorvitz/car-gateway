# Architecture Decision Records

Each file in this folder records one significant design decision: the context, the decision, the alternatives that were considered, and the consequences. When someone asks "why did we do it this way?", the answer is here.

## Rules

- One decision per file, named `NNNN-short-title.md`, numbered in order.
- A new ADR starts as **Proposed** and becomes **Accepted** when its pull request is merged.
- An accepted ADR is never edited to change the decision. To change a decision, write a new ADR and mark the old one **Superseded by NNNN**.

## Index

| ADR | Decision | Status |
| --- | --- | --- |
| [0001](0001-cyclic-executive-single-logic-thread.md) | Cyclic executive at 50 Hz with one logic thread | Accepted |
| [0002](0002-template-method-managers.md) | Managers share one base class (Template Method) | Accepted |
| [0003](0003-shared-buffer-status-and-events.md) | Shared buffer with status and event entries | Accepted |
| [0004](0004-e2e-protection.md) | End-to-end protection on every message | Accepted |
| [0005](0005-no-dynamic-allocation-after-init.md) | No dynamic memory allocation after initialisation | Accepted |
| [0006](0006-cpp17.md) | C++17 | Accepted |
| [0007](0007-e2e-profile.md) | E2E profile: 4-bit alive counter and CRC-8 SAE J1850 | Accepted |

## Template

```markdown
# NNNN. Title

- **Status:** Proposed
- **Date:** YYYY-MM-DD

## Context
What problem or force makes this decision necessary?

## Decision
What we decided, in one or two sentences, then any detail.

## Alternatives considered
Each alternative and why it was not chosen.

## Consequences
What becomes easier, what becomes harder, and what we must now do.
```
