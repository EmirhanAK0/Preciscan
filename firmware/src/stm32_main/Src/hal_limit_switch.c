#include "hal_limit_switch.h"
#include "stm32f4xx_hal.h"

void limit_init(LimitSwitch* sw, LimitPinDef pin_def, uint32_t debounce_ms) {
    sw->pin_def         = pin_def;
    sw->last_raw        = true; // Pull-up varsayimi ile GPIO_PIN_SET (HIGH)
    sw->deb_start_ms    = 0;
    sw->deb_duration_ms = debounce_ms;
    sw->triggered       = false;
}

void limit_update(LimitSwitch* sw, uint32_t now_ms) {
    // STM32'de okuma yapalim
    bool raw = (HAL_GPIO_ReadPin(sw->pin_def.port, sw->pin_def.pin) == GPIO_PIN_SET);
    
    sw->triggered = false; // Her dongu basinda sifirla

    // LOW tetiklenme durumu (Active Low / NO switch + Pull-up)
    if (raw == false) {
        if (sw->last_raw == true) {
            sw->deb_start_ms = now_ms; // Dusme kenari — zamanlayici baslat
        }
        if (now_ms - sw->deb_start_ms > sw->deb_duration_ms) {
            sw->triggered = true; // Debounce suresi gecti
        }
    }

    sw->last_raw = raw;
}

bool limit_is_triggered(const LimitSwitch* sw) {
    return sw->triggered;
}
