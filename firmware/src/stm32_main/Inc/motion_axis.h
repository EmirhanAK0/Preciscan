// ============================================================
// Motion — Eksen Yonetimi (Stepper + Limit + Kinematik)
// ============================================================
#pragma once
#include "hal_stepper.h"
#include "hal_limit_switch.h"
#include "preciscan_config.h"
#include "bsp_pins.h"

// Eksen calisma modu
typedef enum {
    AX_IDLE,
    AX_HOMING,            // Home limitine dogru hareket
    AX_TARGET_MOVE        // Belirli adim sayisi kadar hareket
} AxisMode;

// Eksen olaylari (update donus degeri)
typedef enum {
    AX_NONE,
    AX_HOME_FOUND,        // Home limiti bulundu
    AX_TARGET_REACHED,    // Hedef adima ulasildi
    AX_SAFETY_FAULT       // Guvenlik limiti tetiklendi
} AxisEvent;

typedef struct {
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
} Axis;

void axis_init(Axis* ax,
               MotorPins mp, uint32_t step_us,
               LimitPins lp, bool has_safety,
               float mm_per_step, bool home_dir);

void axis_start_homing(Axis* ax);
void axis_start_move_steps(Axis* ax, long steps, bool direction);
void axis_start_move_mm(Axis* ax, float mm);
void axis_stop(Axis* ax);

/// Her loop() iterasyonunda cagirilir.
/// @param now_ms   hal_millis() degeri  (limit debounce icin)
/// @param now_us   hal_micros() degeri  (step zamanlama icin)
AxisEvent axis_update(Axis* ax, uint32_t now_ms, uint32_t now_us);
