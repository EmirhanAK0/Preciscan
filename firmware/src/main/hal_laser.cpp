#include "hal_laser.h"

void laser_init(LaserTrigger* lt, uint8_t pin, unsigned long pulse_us) {
    lt->pin      = pin;
    lt->active   = false;
    lt->trig_us  = 0;
    lt->pulse_us = pulse_us;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void laser_fire(LaserTrigger* lt) {
    digitalWrite(lt->pin, HIGH);
    lt->active  = true;
    lt->trig_us = micros();
}

void laser_off(LaserTrigger* lt) {
    digitalWrite(lt->pin, LOW);
    lt->active = false;
}

void laser_update(LaserTrigger* lt, unsigned long now_us) {
    if (lt->active && (now_us - lt->trig_us >= lt->pulse_us)) {
        digitalWrite(lt->pin, LOW);
        lt->active = false;
    }
}

bool laser_is_active(const LaserTrigger* lt) {
    return lt->active;
}
