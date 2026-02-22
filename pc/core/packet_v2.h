#pragma once
#include <cstdint>

// Bellekte veri arasına boşluk girmesini engeller (Kritik!)
#pragma pack(push, 1)

struct McuPositionPacket {
    uint32_t trigger_seq;
    float y_position;
};

struct LaserProfilePacket {
    uint32_t trigger_seq;
    float x_data[1280]; // Örnek boyut
    float z_data[1280];
};

#pragma pack(pop)
