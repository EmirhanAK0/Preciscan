// ============================================================
// HAL — Step Motor Surucusu (STM32F4)
// ============================================================
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "bsp_pins.h"

typedef struct {
    MotorPins     pins;
    uint32_t      interval_us;
    uint32_t      last_us;
    bool          enabled;
    bool          direction;      // true = forward
    long          step_count;     // Toplam adim sayaci (yon'a gore +/-)
} StepperDriver;

void stepper_init(StepperDriver* d, MotorPins pins, uint32_t interval_us);
void stepper_set_dir(StepperDriver* d, bool forward);
void stepper_set_interval(StepperDriver* d, uint32_t us);
void stepper_enable(StepperDriver* d);
void stepper_disable(StepperDriver* d);

/// Zamani kontrol eder, gerekiyorsa bir adim atar.
/// @return true ise bir adim atildi.
bool stepper_update(StepperDriver* d, uint32_t now_us);

void stepper_reset_count(StepperDriver* d);
