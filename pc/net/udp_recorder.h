#pragma once
#include <cstdint>
#include <string>

struct SeqTracker;

// Record UDP seq packets (4 bytes each) into a binary file (raw bytes).
// Returns true on success, false on failure (error filled).
bool udp_record_seq_stream(
    int port,
    uint32_t target_count,
    const std::string& out_path,
    SeqTracker& tracker,
    std::string& error
);
