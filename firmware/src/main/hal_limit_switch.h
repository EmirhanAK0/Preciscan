// ============================================================
// HAL — Limit Switch (Debounce destekli)
// ============================================================
#pragma once
#include <Arduino.h>

struct LimitSwitch {
    uint8_t pin;
    bool last_raw;
    unsigned long deb_start_ms;
    unsigned long deb_duration_ms;
    bool triggered;          // Debounce'dan gecmis tetik (her update'te guncellenir)
};

void limit_init(LimitSwitch* sw, uint8_t pin, unsigned long debounce_ms);
void limit_update(LimitSwitch* sw, unsigned long now_ms);
bool limit_is_triggered(const LimitSwitch* sw);
