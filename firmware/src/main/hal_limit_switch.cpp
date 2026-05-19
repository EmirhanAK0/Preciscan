#include "hal_limit_switch.h"

void limit_init(LimitSwitch* sw, uint8_t pin, unsigned long debounce_ms) {
    sw->pin            = pin;
    sw->last_raw       = HIGH;
    sw->deb_start_ms   = 0;
    sw->deb_duration_ms = debounce_ms;
    sw->triggered      = false;

    pinMode(pin, INPUT_PULLUP);
}

void limit_update(LimitSwitch* sw, unsigned long now_ms) {
    bool raw = digitalRead(sw->pin);
    sw->triggered = false;                       // Her dongu basinda sifirla

    if (raw == LOW) {
        if (sw->last_raw == HIGH) {
            sw->deb_start_ms = now_ms;           // Dusme kenari — zamanlayici baslat
        }
        if (now_ms - sw->deb_start_ms > sw->deb_duration_ms) {
            sw->triggered = true;                // Debounce suresi gecti
        }
    }

    sw->last_raw = raw;
}

bool limit_is_triggered(const LimitSwitch* sw) {
    return sw->triggered;
}
