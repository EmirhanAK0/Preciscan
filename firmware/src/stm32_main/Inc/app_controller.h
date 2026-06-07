// ============================================================
// App — Durum Makinesi + Komut Ayristirici (STM32)
// Tum is mantigi ve seri protokol buradan yonetilir.
// ============================================================
#pragma once
#include "motion_axis.h"
#include "scan_engine.h"
#include "uart_comm.h"

// Sistem durumlari (seri protokolde kullanilan isimlerle eslesir)
typedef enum {
    SYS_LIN_HOMING,
    SYS_LIN_POSITIONING,
    SYS_Z_HOMING,
    SYS_Z_MOVING,
    SYS_SCANNING,
    SYS_READY,
    SYS_FAULT
} SystemState;

typedef struct {
    Axis       lin_axis;
    Axis       z_axis;
    ScanEngine scanner;

    SystemState state;
    char        serial_buf[128];   // C tipi string buffer
    float       z_move_mm;         // ZMOVE komutu ile istenen mesafe
} AppController;

void app_init(AppController* app);

/// Her loop() iterasyonunda cagirilir
void app_update(AppController* app, uint32_t now_ms, uint32_t now_us);
