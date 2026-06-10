// ============================================================
// Motion — Tarama Motoru (Rotasyon + Lazer Tetik)
// ============================================================
#pragma once
#include "hal_stepper.h"
#include "hal_laser.h"
#include "preciscan_config.h"

typedef struct {
    StepperDriver rot_motor;
    LaserTrigger  laser;

    float scan_start_angle;      // Tarama baslangicindaki mutlak aci (360 derece durmasi icin)
    float next_trigger_angle;    // Bir sonraki lazer tetik acisi
    float angle_per_trigger;     // Derece cinsinden tetik periyodu
    bool  direction;             // true = CW, false = CCW
    bool  running;
} ScanEngine;

void scan_init(ScanEngine* eng);
void scan_start(ScanEngine* eng, bool cw);
void scan_stop(ScanEngine* eng);

/// Her loop() iterasyonunda cagirilir.
/// @param now_us   hal_micros() degeri
/// @param angle_out   Tetik olustuysa aci degeri buraya yazilir
/// @return true ise lazer tetiklendi ve angle_out gecerli
bool scan_update(ScanEngine* eng, uint32_t now_us, float* angle_out);

/// Lazerin suresi doldu mu kontrolu (loop'tan ayrica cagirilir)
void scan_laser_update(ScanEngine* eng, uint32_t now_us);
