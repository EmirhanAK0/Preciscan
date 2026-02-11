\# PC Thread Model \& Dataflow Specification



Status: Draft (M1)



Goal: Ensure high-throughput UDP acquisition without UI-induced packet loss.



---



\## 1. Threads (Hard Separation)



\### 1) Acquisition Thread (Highest Priority)

Responsibilities:

\- Receive UDP packets

\- Validate header fields (magic/version)

\- Track sequence (`seq`) and compute PLR metrics

\- Push raw payload into an SPSC ring buffer

Rules:

\- Must never block on UI

\- Must avoid heavy allocations

\- Must not perform expensive processing



Outputs:

\- RawPacketQueue (SPSC) → Processing thread

\- Telemetry counters (atomic)



---



\### 2) Processing Thread (Medium Priority)

Responsibilities:

\- Pop packets from RawPacketQueue

\- Decode payload to measurement samples

\- Apply lightweight filters (optional)

\- Coordinate transforms (polar→Cartesian etc.)

\- Produce:

&nbsp; - Preview stream (downsampled)

&nbsp; - Full-resolution dataset (to disk or memory-mapped file)



Rules:

\- Can drop preview frames if overloaded

\- Must not block Acquisition thread



Outputs:

\- PreviewQueue → UI thread (bounded)

\- DatasetWriter → persistent storage



---



\### 3) UI / Render Thread (Lowest Priority)

Responsibilities:

\- Display preview

\- Operator controls (start/stop/config)

Rules:

\- UI must be considered "best effort"

\- UI must never control Acquisition timing

\- If UI lags, preview drops are acceptable



---



\## 2. Queues \& Backpressure Policy



\### 2.1 RawPacketQueue (Acquisition → Processing)

\- Type: Single Producer Single Consumer (SPSC) ring buffer

\- Capacity: configurable (e.g., 256–4096 packets)

\- Overflow policy: if full, increment `drop\_due\_to\_overflow` and drop newest (or oldest) depending on chosen strategy (TBD)



\### 2.2 PreviewQueue (Processing → UI)

\- Bounded queue

\- Overflow policy: drop oldest (keep UI responsive)



---



\## 3. Required Metrics (per run\_id)



Acquisition metrics:

\- rx\_packets

\- missing\_packets (seq gaps)

\- ooo\_packets (out-of-order)

\- dup\_packets

\- overflow\_drops

\- computed PLR



Processing metrics:

\- decode\_time\_ms (avg/max)

\- processing\_fps (preview)

\- queue\_depths (raw/preview)



UI metrics:

\- render\_fps



All metrics must be logged with:

\- run\_id

\- firmware\_version

\- protocol\_version

\- config\_hash



---



\## 4. Recording / Replay (Mandatory for Debug)



The PC must support:

\- recording raw UDP packets to disk (binary log)

\- replaying recorded streams into the pipeline



Purpose:

\- regression testing without hardware

\- performance benchmarking



---



\## 5. Non-Goals (For M1)



\- No full-quality meshing requirement in realtime

\- Online visualization is preview-only



