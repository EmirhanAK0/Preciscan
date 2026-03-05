# Preciscan Coding Standard v1.0

## 1. Architecture Layers

| Layer      | Responsibility |
|------------|---------------|
| `core`     | Pure business logic, data processing, protocol parsing/validation, domain types |
| `net`      | Network/transport layer (TCP/UDP/socket), byte stream management |
| `io`       | Raw data I/O — UDP recording/reading (`udp_recorder`, `udp_receiver`), MCU listening (`mcu_listener`), ring buffer infrastructure. Below `net`, not dependent on `core` |
| `app`      | Application orchestration (`main`, config, lifecycle, wiring) |
| `sim`      | Simulation, replay or fake data sources |
| `hardware` | Hardware adapters |

---

## 2. Layer Dependency Rules

Dependencies are **strictly one-directional**:
```
app  ->  net  ->  io  ->  core
app  ->  sim       ->  core
app  ->  hardware  ->  core
```

### Forbidden
```
core -> net
core -> io
core -> app
net  -> app
io   -> app
io   -> net
```

### Core Principle

`core` has **no knowledge of**: network, socket, file system, OS API, `io` or `net` layers.

`core` only knows: data, algorithms, protocol logic.

---

## 3. Data Flow Principle

Core never pulls data from the outside world. Data is pushed into `core` by outer layers.
```
UDP socket  ->  io  ->  net  ->  core  ->  app
file replay ->  sim ->  app  ->  core
```

---

## 4. File Organization

### File Names

`snake_case` is used for all file names.
```
packet_parser.cpp
packet_parser.h
seq_metrics.cpp
mcu_listener.cpp
udp_recorder.cpp
```

### Header Guard

Every header file uses `#pragma once`.

### Include Order

1. Own header
2. Standard library
3. Third-party
4. Project headers
```cpp
#include "packet_parser.h"

#include <vector>
#include <string>

#include <fmt/core.h>

#include "core/result.h"
```

### Include Path Convention

Project headers are prefixed with the layer name:
```cpp
#include "core/result.h"
#include "net/udp_socket.h"
#include "io/udp_recorder.h"
```

---

## 5. Naming Conventions

| Element          | Rule        | Example                        |
|------------------|-------------|--------------------------------|
| Class / Struct   | PascalCase  | `PacketParser`, `ScanResult`   |
| Function         | camelCase   | `parsePacket()`, `computeChecksum()` |
| Variable         | camelCase   | `packetSize`, `scanCount`      |
| Member variable  | camelCase`_`| `packetSize_`, `checksum_`     |
| Constant         | `kPascalCase` | `kMaxPacketSize`, `kTimeoutMs` |
| Namespace        | lowercase   | `preciscan`, `preciscan::core` |
| File             | snake_case  | `packet_parser.cpp`            |

---

## 6. CMake Target Naming

Each layer has its own CMake target and `CMakeLists.txt`:
```
preciscan_core
preciscan_net
preciscan_hw
preciscan_ui
```

---

## 7. Code Format Standard

Tool: **clang-format** (mandatory, configuration in `.clang-format` at repo root)

| Rule         | Value    |
|--------------|----------|
| Indent       | 4 spaces |
| Brace style  | Allman   |
| Column limit | 100      |
```cpp
if (ready)
{
    run();
}
```

`third_party/` is excluded from formatting.

---

## 8. C++ Usage Rules

### Required

- `nullptr` (never `NULL`)
- `enum class` preferred over plain `enum`
- `static_cast` (C-style cast is forbidden)
- RAII principles must be followed

### Forbidden
```cpp
// FORBIDDEN in header files
using namespace std;
using namespace preciscan;
```

---

## 9. Error Handling

No exceptions in `core`.

Functions that process external input return `Result<T>` or `Status`:
```cpp
Result<Packet> parsePacket(Bytes bytes);
Status validateChecksum(uint32_t checksum);
```

Error cases: `ParseError`, `InvalidInput`, `Timeout`, `ChecksumError`.

`core` only reports the error. `app` or `net` decides how to handle it.

---

## 10. Logging Standard

No `printf` or `std::cout`. A single logging infrastructure is used.

Log messages must contain: timestamp, packet id, sequence number, error code (if any).

---

## 11. Concurrency Rules

Every module must document:

- Is it thread-safe?
- Single producer / single consumer?
- Which thread calls it?
```cpp
// Thread safety: SPSC — single producer (io thread), single consumer (app thread).
// Lock-free. Two producers or two consumers must never be used simultaneously.
class SpscRingBuffer { ... };
```

---

## 12. Test Standard

Framework: **Catch2**

Test priority:
```
core  >  net  >  io  >  app
```

Must be tested: packet parsing, metrics, boundary conditions, invalid/malformed input, checksum errors.

---

## Summary

| Rule             | Value           |
|------------------|-----------------|
| Function/variable | camelCase      |
| Class/Struct     | PascalCase      |
| Member variable  | `value_`        |
| Constant         | `kPascalCase`   |
| File             | snake_case      |
| Namespace        | lowercase       |
| Indent           | 4 spaces        |
| Brace style      | Allman          |
| Column limit     | 100             |
| Core dependency  | Independent     |
| Error handling   | `Result<T>` / `Status` |
| Format tool      | clang-format (mandatory) |
| Test framework   | Catch2          |
