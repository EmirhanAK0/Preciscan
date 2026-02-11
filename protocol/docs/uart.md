\# UART Control Protocol (Draft)



This document defines the UART control channel between \*\*PC (Master)\*\* and \*\*MCU (Slave)\*\*.



Status: Draft (M1)

Framing rules: see framing.md”



“Message schema: see schema/commands.proto”





---



\## 1. Transport



\- Link: UART

\- Direction: bidirectional

\- Purpose: control commands + telemetry + safety heartbeat



---



\## 2. Framing (TBD)



Binary framing and CRC will be defined in M1.2.

For now, message types and fields are defined at a logical level.



---



\## 3. Message Types



\### 3.1 PC → MCU Commands



| Command | Purpose | Required Fields |

|--------|---------|-----------------|

| `PING` | link check | - |

| `HEARTBEAT` | keepalive | `seq` |

| `ESTOP` | immediate stop | - |

| `CLEAR\_FAULT` | clear error state | - |

| `HOME` | homing sequence | - |

| `MOVE\_ABS` | move to absolute position | `pos` `vel` `acc` |

| `MOVE\_REL` | relative move | `delta` `vel` `acc` |

| `JOG` | manual jog | `dir` `vel` |

| `SCAN\_START` | start scan mode | `scan\_id` `params` |

| `SCAN\_STOP` | stop scan mode | - |

| `SET\_PARAM` | update parameter | `key` `value` |

| `GET\_STATUS` | request status | - |



---



\### 3.2 MCU → PC Telemetry



| Message | Purpose | Fields |

|--------|---------|--------|

| `PONG` | response to PING | `fw\_version` |

| `STATUS` | periodic status report | `state` `pos` `vel` `fault` |

| `FAULT` | fault report | `fault\_code` `details` |

| `HEARTBEAT\_ACK` | ack keepalive | `seq` |

| `ENCODER` | optional streaming | `angle` `valid` |

| `LIMIT\_HIT` | limit switch event | `which` |



---



\## 4. State Machine (MCU)



States (draft):

\- `IDLE`

\- `HOMING`

\- `READY`

\- `SCANNING`

\- `FAULT`

\- `ESTOP`



---



\## 5. Notes



\- MCU must enter `FAULT` on heartbeat timeout.

\- `ESTOP` must be handled with highest priority.





