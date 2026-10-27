# Motor Controller ECU: Serial Protocol (Interface Specification)

The ECU is a C program that behaves like firmware on a microcontroller. It reads command frames on `stdin` (standing in for UART RX) and writes response frames on `stdout` (UART TX). The Python test suite plays the role of a test bench or HIL rig.

## Frame format

```
| 0xAA | CMD | LEN | PAYLOAD (LEN bytes) | CRC8 |
```

- CRC-8 over `CMD, LEN, PAYLOAD`: polynomial 0x31, init 0xFF
- Integers are big-endian
- Bytes before a `0xAA` start byte are discarded (resynchronization)
- Response: same format, `CMD | 0x80`, first payload byte is the status

## Commands

| CMD | Name | Request payload | Response payload |
|---|---|---|---|
| 0x01 | SET_SPEED | `int16` rpm, allowed 0 … 3000 | `[status]` |
| 0x02 | GET_STATUS | none | `[status, state, rpm_hi, rpm_lo]` |
| 0x03 | ESTOP | none | `[status]` |
| 0x04 | RESET | none | `[status]` |

## States

| Value | State | Meaning |
|---|---|---|
| 0 | IDLE | motor stopped |
| 1 | RUNNING | speed > 0 |
| 2 | ESTOP | emergency stop latched, speed forced to 0 |

## Status codes

| Code | Meaning |
|---|---|
| 0x00 | OK |
| 0x01 | CRC error |
| 0x02 | unknown command |
| 0x03 | wrong payload length |
| 0x04 | value out of range |
| 0x05 | rejected in current state (e.g. SET_SPEED during ESTOP, RESET when not in ESTOP) |

## Behavioral requirements

| ID | Requirement |
|---|---|
| IF-01 | After power-up the ECU is IDLE with speed 0. |
| IF-02 | SET_SPEED with 1…3000 rpm sets RUNNING; 0 rpm sets IDLE. |
| IF-03 | SET_SPEED outside 0…3000 is rejected with 0x04, state unchanged. |
| IF-04 | A frame with a bad CRC is answered with 0x01 and has no effect. |
| IF-05 | Unknown commands are answered with 0x02; wrong lengths with 0x03. |
| IF-06 | ESTOP forces speed 0 and state ESTOP from any state. |
| IF-07 | In ESTOP, SET_SPEED is rejected with 0x05 until RESET. |
| IF-08 | RESET leaves ESTOP to IDLE; RESET outside ESTOP is rejected with 0x05. |
| IF-09 | The ECU resynchronizes on garbage bytes and handles frames split across or packed into reads. |
| IF-10 | The ECU never crashes or stops responding on malformed input. |
