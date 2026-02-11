\# Run Logging \& Dataset Specification



Status: Draft (M1)



Goal: Every scan/run must be reproducible and traceable.



---



\## 1. Run Identity



Each scan session generates a unique:

\- `run\_id` (UUID recommended)



A run is a folder on disk:





---



\## 2. Mandatory Files per Run



\### 2.1 `meta.json`

Must include:

\- run\_id

\- timestamp\_start / timestamp\_end (ISO 8601)

\- operator (optional)

\- pc\_app\_version (git tag + commit hash)

\- firmware\_version

\- protocol\_version

\- machine\_id (optional)

\- notes (optional)



\### 2.2 `config.json`

Snapshot of all parameters used:

\- motion parameters (vel/acc limits, homing params)

\- scan plan (z range, step, angular step, etc.)

\- sensor parameters (if any)

\- filtering parameters (if any)



\### 2.3 `metrics.json`

Collected counters and performance metrics:

\- rx\_packets, missing\_packets, ooo\_packets, dup\_packets

\- overflow\_drops

\- PLR

\- avg/max decode time

\- preview FPS (if available)



\### 2.4 Raw packet recording (optional but recommended for debug)

\- `udp\_raw.bin` (binary stream) OR `udp\_raw.pcap`



---



\## 3. Data Products



\### 3.1 Preview

\- `preview.ply` (downsampled point cloud)



\### 3.2 Full Resolution

At least one of:

\- `cloud\_full.ply` (point cloud)

\- `cloud\_full.bin` (custom efficient format, TBD)



Optionally:

\- `mesh.stl` (post-processed)



---



\## 4. Logging Rules



\- PC application logs to:

&nbsp; - `app.log` (text)

&nbsp; - optionally `app.jsonl` (structured per-line JSON)



Log fields MUST include:

\- timestamp

\- level

\- module

\- run\_id (if within a run)

\- message

\- key-value context



---



\## 5. Failure Handling (Non-Negotiable)



If a run is aborted or fails:

\- still write `meta.json` and `metrics.json`

\- include failure reason and last known state



Reason: failed runs are valuable evidence.



---



