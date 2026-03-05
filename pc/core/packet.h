#pragma once
#include <cstdint>
#include <vector>

// Tek ve yetkili Packet tanimi - tum moduller buradan kullanir
struct Packet
{
    std::vector<uint8_t> data;
    // uint64_t timestamp_ns; // Ileride aktif edilecek
};
