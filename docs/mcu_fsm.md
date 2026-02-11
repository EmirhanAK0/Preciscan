\# MCU State Machine (FSM) Specification



Status: Draft (M1)



This document specifies the MCU (Slave) runtime behavior as a deterministic finite state machine.



---



\## 1. States



\### IDLE

\- Motors disabled or holding torque (configurable)

\- Accepts: PING, HEARTBEAT, GET\_STATUS, HOME, CLEAR\_FAULT

\- Rejects: MOVE/SCAN commands (unless explicitly allowed)



\### HOMING

\- Runs homing routine (limit switch based)

\- On success → READY

\- On failure (timeout / limit fault) → FAULT

\- Accepts: ESTOP, HEARTBEAT



\### READY

\- Homed and ready for motion / scan

\- Accepts: MOVE\_ABS, MOVE\_REL, JOG, SCAN\_START, GET\_STATUS, SET\_PARAM, ESTOP, HEARTBEAT



\### SCANNING

\- Scan mode active

\- Motion is constrained by scan plan

\- Position-synchronized trigger generation enabled (if configured)

\- Accepts: SCAN\_STOP, ESTOP, HEARTBEAT, GET\_STATUS

\- Rejects: direct MOVE commands (prevents unsafe operator interference)



\### FAULT

\- Motion output disabled immediately

\- Trigger output disabled

\- Requires explicit CLEAR\_FAULT to leave FAULT state

\- Accepts: CLEAR\_FAULT, GET\_STATUS, PING, HEARTBEAT

\- Rejects: MOVE/SCAN/HOME



\### ESTOP

\- Highest priority emergency stop state

\- Motor disable + trigger disable immediately

\- Only leaves ESTOP on: CLEAR\_FAULT + explicit reset condition (TBD)

\- Accepts: PING, GET\_STATUS, HEARTBEAT, CLEAR\_FAULT (may be gated)



---



\## 2. Global Safety Rules



\- ESTOP command MUST be handled immediately in any state.

\- Heartbeat timeout:

&nbsp; - If no valid HEARTBEAT is received within `T\_HB\_TIMEOUT`, enter FAULT.

\- Limit switch hit:

&nbsp; - During HOMING: expected limit hit is allowed.

&nbsp; - During READY/SCANNING: unexpected limit hit enters FAULT and disables motion.



---



\## 3. Heartbeat Timing (Draft)



\- PC sends HEARTBEAT every 200 ms

\- MCU timeout: 500 ms without valid heartbeat → FAULT



---



\## 4. Acceptance Criteria (Done Definition)



A firmware implementation is accepted when:



1\) \*\*State correctness\*\*

\- All transitions follow this document

\- Invalid commands are rejected with a FAULT or NACK (TBD) without undefined behavior



2\) \*\*Safety\*\*

\- ESTOP always stops outputs in < 10 ms (target)

\- Heartbeat timeout reliably triggers FAULT



3\) \*\*Observability\*\*

\- MCU provides STATUS telemetry including:

&nbsp; - current state

&nbsp; - position

&nbsp; - last fault code



4\) \*\*Determinism\*\*

\- SCANNING behavior does not depend on UART timing jitter



---



\## 5. Open Decisions (M1.5)



\- Exact homing sequence (direction, speed, backoff)

\- Whether IDLE allows MOVE (operator convenience vs safety)

\- ESTOP recovery policy



