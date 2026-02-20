#pragma once
#include <vector>
#include <cstdint>

// 1. MCU'dan gelecek olan Konum Paketi
struct McuPositionPacket {
    uint32_t trigger_seq; 
    float    y_position;  
    uint8_t  status;      
};

// 2. Micro-Epsilon'dan gelecek olan Profil Paketi
struct LaserProfilePacket {
    uint32_t trigger_seq; 
    uint32_t point_count; 
    std::vector<double> x_points; 
    std::vector<double> z_points; 
};

// 3. Füzyon Sonucu
struct PointCloudScanline {
    uint32_t trigger_seq;
    std::vector<double> x; 
    float y;               
    std::vector<double> z; 
};
