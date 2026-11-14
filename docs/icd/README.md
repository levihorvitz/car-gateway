# Interface Control Document (ICD)

`icd/car_gateway.dbc` is the communication matrix of the system: every message on the CAN bus, who sends it, who receives it, how often, and the layout of every signal. It is the contract between the central computer, the vehicle units, the simulated car and the dashboard.

## Rules

- The DBC file is the single source of truth. Code that encodes or decodes messages is generated from it or checked against it, never written from memory.
- Every change starts as a change request: a GitHub issue titled `CR: <summary>`, stating what changes, why, and which units are affected. Both developers agree in the issue before the pull request is opened.
- The pull request changes the DBC file and bumps its `VERSION`, and needs the approval of both developers.
- CI checks every change with `cantools`: the file must load, no signals may overlap, and every signal must fit in its message.

## Versioning

The `VERSION` line in the DBC file follows semantic versioning:

| Change | Version part | Example |
| --- | --- | --- |
| A signal or message is removed, moved, resized or rescaled | Major | `0.1.0` to `1.0.0` |
| A new message or a new signal in free bits | Minor | `0.1.0` to `0.2.0` |
| Comments or attributes only | Patch | `0.1.0` to `0.1.1` |

A major change breaks every receiver of the affected message, so its change request lists them all.

## Conventions

- 500 kbit/s, standard 11-bit IDs, little-endian (Intel) signals.
- Lower IDs win arbitration: status from the units uses `0x1xx`, messages from the central computer use `0x2xx`.
- Every message reserves the alive counter and the CRC in bytes 6 and 7 (ADR 0007).

## Tools

```bash
pipx install cantools
cantools dump icd/car_gateway.dbc
```

`cantools dump` prints every message with a diagram of its bit layout. The DBC file is plain text and can be edited directly, or in a graphical editor such as SavvyCAN or the Kvaser Database Editor.
