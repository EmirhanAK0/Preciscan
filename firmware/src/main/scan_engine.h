// ============================================================
// Motion — Tarama Motoru (Rotasyon + Lazer Tetik)
// ============================================================
#pragma once
#include "hal_stepper.h"
#include "hal_laser.h"
#include "preciscan_config.h"
#include <math.h>

struct ScanEngine {
    StepperDriver rot_motor;
    LaserTrigger  laser;

    float step_accum;            // Tetik icin adim biriktirici
    float steps_per_trigger;     // Kac adimda bir lazer tetikle
    long  total_steps;           // Toplam atilan adim (yonlu)
    bool  direction;             // true = CW
    bool  running;
};

void scan_init(ScanEngine* eng);
void scan_start(ScanEngine* eng, bool cw);
void scan_stop(ScanEngine* eng);

/// Her loop() iterasyonunda cagirilir.
/// @param now_us   micros() degeri
/// @param angle_out   Tetik olustuysa aci degeri buraya yazilir
/// @return true ise lazer tetiklendi ve angle_out gecerli
bool scan_update(ScanEngine* eng, unsigned long now_us, float* angle_out);

/// Lazerin suresi doldu mu kontrolu (loop'tan ayrica cagirilir)
void scan_laser_update(ScanEngine* eng, unsigned long now_us);
