#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "bsp_pins.h"

typedef struct {
    LimitPinDef pin_def;
    bool        last_raw;
    uint32_t    deb_start_ms;
    uint32_t    deb_duration_ms;
    bool        triggered;
} LimitSwitch;

void limit_init(LimitSwitch* sw, LimitPinDef pin_def, uint32_t debounce_ms);
void limit_update(LimitSwitch* sw, uint32_t now_ms);
bool limit_is_triggered(const LimitSwitch* sw);
