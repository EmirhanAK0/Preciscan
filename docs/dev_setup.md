\# Development Environment Setup



This document describes the required development environment for Preciscan.



---



\## 1. Operating System



Currently tested on:

\- Windows 11



---



\## 2. Required Tools



\### General

\- Git

\- CMake (>= 3.20 recommended)

\- A C++17 compatible compiler (for future PC core modules)



\### Firmware



The firmware implementation strategy is under evaluation.



The architecture is designed to support:

\- Deterministic motion control

\- Encoder-based triggering

\- Safety state machine

\- Hardware abstraction layer



Language and toolchain decisions will be finalized in M1.



---



\### PC Application



Technology stack will be defined in M1.



The system is expected to support:

\- High-throughput UDP acquisition

\- Lock-free buffering

\- Modular processing pipeline

\- Structured logging



---



