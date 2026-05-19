// ============================================================
// HAL — Step Motor Surucusu
// ============================================================
#pragma once
#include <Arduino.h>

struct StepperDriver {
    uint8_t step_pin;
    uint8_t dir_pin;
    unsigned long interval_us;
    unsigned long last_us;
    bool enabled;
    bool direction;      // true = forward
    long step_count;     // Toplam adim sayaci (yon'a gore +/-)
};

void stepper_init(StepperDriver* d, uint8_t step_pin, uint8_t dir_pin,
                  unsigned long interval_us);
void stepper_set_dir(StepperDriver* d, bool forward);
void stepper_set_interval(StepperDriver* d, unsigned long us);
void stepper_enable(StepperDriver* d);
void stepper_disable(StepperDriver* d);

/// Zamani kontrol eder, gerekiyorsa bir adim atar.
/// @return true ise bir adim atildi.
bool stepper_update(StepperDriver* d, unsigned long now_us);

void stepper_reset_count(StepperDriver* d);
