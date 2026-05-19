// ============================================================
// App — Durum Makinesi + Komut Ayristirici
// Tum is mantigi ve seri protokol buradan yonetilir.
// ============================================================
#pragma once
#include "motion_axis.h"
#include "scan_engine.h"

// Sistem durumlari (seri protokolde kullanilan isimlerle eslesir)
enum SystemState : uint8_t {
    SYS_LIN_HOMING,
    SYS_LIN_POSITIONING,
    SYS_Z_HOMING,
    SYS_Z_MOVING,
    SYS_SCANNING,
    SYS_READY,
    SYS_FAULT
};

struct AppController {
    Axis       lin_axis;
    Axis       z_axis;
    ScanEngine scanner;

    SystemState state;
    String      serial_buf;
    float       z_move_mm;         // ZMOVE komutu ile istenen mesafe (raporlama icin)
};

void app_init(AppController* app);

/// Her loop() iterasyonunda cagirilir
void app_update(AppController* app, unsigned long now_ms, unsigned long now_us);
