\# Packet Replay Tool (Skeleton)



Status: Draft (M1)



Purpose:

Replay recorded UDP packet streams into the PC acquisition pipeline without requiring hardware.



---



\## Inputs (planned)



\- `udp\_raw.bin` recorded during a run (see `docs/run\_logging.md`)

&nbsp; OR

\- `.pcap` capture file (optional)



---



\## Core Features (planned)



\- Read packets in original order

\- Support configurable replay rate:

&nbsp; - real-time

&nbsp; - accelerated

&nbsp; - step-by-step

\- Preserve original `seq` values to validate PLR logic

\- Output statistics:

&nbsp; - total packets

&nbsp; - missing / out-of-order / duplicates (if present in recording)



---



\## Usage (planned)



```powershell

\# Example (to be implemented)

packet\_replay.exe --input runs/<run\_id>/udp\_raw.bin --rate realtime



