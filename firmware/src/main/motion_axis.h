// ============================================================
// Motion — Eksen Yonetimi (Stepper + Limit + Kinematik)
// ============================================================
#pragma once
#include "hal_stepper.h"
#include "hal_limit_switch.h"
#include "preciscan_config.h"
#include "bsp_pins.h"

// Eksen calisma modu
enum AxisMode : uint8_t {
    AX_IDLE,
    AX_HOMING,            // Home limitine dogru hareket
    AX_TARGET_MOVE        // Belirli adim sayisi kadar hareket
};

// Eksen olaylari (update donus degeri)
enum AxisEvent : uint8_t {
    AX_NONE,
    AX_HOME_FOUND,        // Home limiti bulundu
    AX_TARGET_REACHED,    // Hedef adima ulasildi
    AX_SAFETY_FAULT       // Guvenlik limiti tetiklendi
};

struct Axis {
    StepperDriver motor;
    LimitSwitch home_limit;
    LimitSwitch safety_limit;
    bool has_safety;           // Guvenlik limiti var mi?

    float mm_per_step;
    bool home_direction;       // Home yonu

    AxisMode mode;
    bool homed;
    long target_steps;         // Hedef icin geri sayim
    long position_steps;       // Home'dan itibaren konum
};

void axis_init(Axis* ax,
               const MotorPins& mp, unsigned long step_us,
               const LimitPins& lp, bool has_safety,
               float mm_per_step, bool home_dir);

void axis_start_homing(Axis* ax);
void axis_start_move_steps(Axis* ax, long steps, bool direction);
void axis_start_move_mm(Axis* ax, float mm);
void axis_stop(Axis* ax);

/// Her loop() iterasyonunda cagirilir.
/// @param now_ms   millis() degeri  (limit debounce icin)
/// @param now_us   micros() degeri  (step zamanlama icin)
AxisEvent axis_update(Axis* ax, unsigned long now_ms, unsigned long now_us);
