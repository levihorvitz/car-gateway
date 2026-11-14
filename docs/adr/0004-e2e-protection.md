# 0004. End-to-end protection on every message

- **Status:** Accepted
- **Date:** 2026-11-11

## Context

The system must detect a lost, repeated or corrupted message from any unit, and must never act on such a message (`SYS_MON_003`, `SYS_MON_004`). CAN has its own CRC on the wire, but it does not catch a message that a sender never sent, a sender that keeps repeating a stale message, or corruption inside a sender or receiver.

## Decision

Every message defined in the DBC file carries an alive counter and a CRC computed over its payload. The sender increments the counter on every send. The receiving manager checks both before using the content:

- Counter skipped: one or more messages were lost.
- Counter unchanged: the message is repeated (stale).
- CRC mismatch: the message is corrupted, and its content is ignored.

This follows the idea of end-to-end protection used in automotive software (for example AUTOSAR E2E), in a simplified form.

## Alternatives considered

- **Rely on the CAN CRC only.** Catches bit errors on the wire, but not lost, repeated or internally corrupted messages.
- **Timeout only.** Detects a silent unit, but not a unit that keeps sending the same stale value.

## Consequences

- Every message loses some bytes of payload to the counter and the CRC; the DBC layout must reserve them.
- The simulated units must implement the same protection, which also lets fault injection corrupt messages on purpose.
- E2E checks are part of the manager base class, so every unit gets them.
- On the real vehicle, standard OBD-II responses do not carry this protection; they are checked by timeout and range only.
