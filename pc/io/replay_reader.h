#pragma once
#include <cstdint>
#include <string>

class SeqTracker;

// Reads raw file of uint32 (network byte order) and updates tracker.
// Returns true on success, false on failure (error filled).
bool replay_seq_file(
    const std::string& in_path,
    uint32_t target_count,         // 0 => read all
    SeqTracker& tracker,
    std::string& error
);
