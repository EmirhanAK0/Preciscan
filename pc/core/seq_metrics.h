#pragma once
#include <cstdint>

struct SeqStats {
    uint64_t rx = 0;
    uint64_t missing = 0;
    uint64_t ooo = 0;
    uint64_t dup = 0;
};

struct SeqTracker {
    bool has_last = false;
    uint32_t last = 0;
    SeqStats stats{};

    void update(uint32_t seq);
    double plr() const; // missing / (rx + missing)
};
