#pragma once
#include <cstdint>
#include <string>

struct SeqTracker;

// Returns true on success, false on failure (error filled)
bool udp_receive_seq_stream(
    int port,
    uint32_t target_count,
    SeqTracker& tracker,
    std::string& error
);
