\# UDP Data Protocol (Draft)



This document defines the UDP payload contract for high-throughput sensor data streaming to the PC.



Status: Draft (M1)



---



\## 1. Transport



\- Link: UDP over Ethernet (GigE)

\- Direction: Sensor → PC

\- Purpose: high-bandwidth measurement stream



---



\## 2. Goals



\- Detect packet loss (sequence tracking)

\- Support position-synchronized scanning (encoder tagging)

\- Allow offline replay/debug (recording raw packets)



---



\## 3. Packet Layout (Logical)



Each UDP datagram contains:



\### 3.1 Header (fixed-size)

| Field | Type | Description |

|------|------|-------------|

| `magic` | u32 | constant identifier (e.g., 0x50524353) |

| `version` | u16 | protocol version |

| `header\_len` | u16 | bytes |

| `seq` | u32 | monotonically increasing packet sequence |

| `device\_ts\_us` | u64 | device timestamp (microseconds), if available |

| `frame\_id` | u32 | scan frame / sweep id |

| `flags` | u32 | bit flags (e.g., valid, overflow, etc.) |

| `payload\_len` | u32 | bytes |

| `crc32` | u32 | header+payload CRC (optional, TBD) |



\### 3.2 Position Tag (optional but recommended)

For position-based acquisition, packets MAY include:



| Field | Type | Description |

|------|------|-------------|

| `encoder\_angle\_raw` | u16/u32 | raw encoder reading |

| `encoder\_angle\_rad` | f32 | angle in radians (if computed on device) |

| `z\_pos\_um` | i32 | optional Z position tag (micrometers) |



---



\## 4. Payload Types



`flags` or a `payload\_type` field (TBD) selects payload interpretation.



\### 4.1 Point-like payload (common)

A payload contains N measurements:



| Field | Type | Description |

|------|------|-------------|

| `N` | u16/u32 | number of samples |

| `samples\[i].distance\_mm` | f32 | measured distance |

| `samples\[i].intensity` | u16 | optional intensity |

| `samples\[i].quality` | u8 | optional quality flag |



\### 4.2 Raw profile payload (alternative)

Device-specific raw profile bytes (for future decoding).

Defined later if needed.



---



\## 5. Packet Loss Rate (PLR)



PC must compute:

\- missing packets from `seq` gaps

\- out-of-order packets (seq decreases)

\- duplicates



Recommended counters (per run):

\- `rx\_packets`

\- `missing\_packets`

\- `dup\_packets`

\- `ooo\_packets`

\- `plr = missing\_packets / (rx\_packets + missing\_packets)`



---



\## 6. Notes / TBD



\- Exact byte layout, endianness, and CRC policy will be finalized after capturing real packets.

\- If encoder tagging is not available in UDP stream, PC must rely on UART telemetry or MCU-provided tags.



