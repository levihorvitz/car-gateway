# 0007. E2E profile: 4-bit alive counter and CRC-8 SAE J1850

- **Status:** Proposed
- **Date:** 2026-11-14

## Context

ADR 0004 decided that every message carries an alive counter and a CRC. This ADR decides the exact profile, so that every sender and receiver, including the simulated car, computes and checks them the same way.

## Decision

Every message in `icd/car_gateway.dbc` reserves:

- **Alive counter:** 4 bits, start bit 48 (byte 6, low nibble). The sender increments it by 1 on every send and wraps from 15 to 0.
- **CRC:** 8 bits, start bit 56 (byte 7). CRC-8 SAE J1850: polynomial `0x1D`, initial value `0xFF`, final XOR `0xFF`. It is computed over the 11-bit message ID as two bytes (low byte first), followed by payload bytes 0 to 6.

The receiver, for each message:

| Check | Result | Meaning |
| --- | --- | --- |
| CRC does not match | Discard the content | Corrupted message |
| Counter = previous + 1 (mod 16) | Accept | Normal |
| Counter = previous | Discard the content | Repeated (stale) message |
| Counter > previous + 1 (mod 16) | Accept, report lost messages | One or more messages lost |

## Alternatives considered

- **AUTOSAR E2E profiles as specified.** The industry reference. Their full state machines and configuration are more than this project needs; this profile keeps the same idea in a simpler form.
- **CRC-16.** Stronger, but costs a second byte in every message; CRC-8 is enough for 8-byte messages at this rate.
- **CRC over the payload only.** Would not catch a message that arrives with the wrong ID. Including the ID costs nothing on the wire.

## Consequences

- Payload bytes 6 (high nibble) and 7 are not available for signals in any message.
- The CRC algorithm is written once and shared by the central computer and the simulated car, with unit tests against known values.
- A 4-bit counter cannot tell 1 lost message from 17. At 20 ms per message, 16 messages are 320 ms, longer than the 100 ms after which a silent unit is already reported (`SYS_MON_002`).
