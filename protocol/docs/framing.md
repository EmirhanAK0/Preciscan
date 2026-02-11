\# Message Framing \& Integrity (Draft)



This document defines framing and integrity rules for UART messages.



Status: Draft (M1)



---



\## 1. Framing



UART messages use:

\- COBS encoding

\- 0x00 as packet delimiter



Rationale:

\- Robust resynchronization after byte errors

\- No ambiguous start/end markers



---



\## 2. Integrity



Each decoded frame contains:

\- `seq` (u16 or u32): monotonically increasing message sequence

\- `msg\_type` (u16): message type id

\- `payload\_len` (u16)

\- `payload` (bytes)

\- `crc16` (u16): CRC16-CCITT over (seq + msg\_type + payload\_len + payload)



---



\## 3. Handling Rules



\- If CRC fails: drop frame, increment counter.

\- If seq jumps: record telemetry (possible loss).

\- Heartbeat timeout must force MCU into FAULT state.



