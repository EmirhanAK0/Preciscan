#include "hal_stepper.h"
#include "bsp_pins.h"

void stepper_init(StepperDriver* d, uint8_t step_pin, uint8_t dir_pin,
                  unsigned long interval_us) {
    d->step_pin    = step_pin;
    d->dir_pin     = dir_pin;
    d->interval_us = interval_us;
    d->last_us     = micros();
    d->enabled     = false;
    d->direction   = true;
    d->step_count  = 0;

    pinMode(step_pin, OUTPUT);
    pinMode(dir_pin,  OUTPUT);
    stepper_set_dir(d, true);
}

void stepper_set_dir(StepperDriver* d, bool forward) {
    d->direction = forward;
    digitalWrite(d->dir_pin, forward ? BSP_DIR_FORWARD : BSP_DIR_REVERSE);
}

void stepper_set_interval(StepperDriver* d, unsigned long us) {
    d->interval_us = us;
}

void stepper_enable(StepperDriver* d) {
    d->enabled = true;
    d->last_us = micros();
}

void stepper_disable(StepperDriver* d) {
    d->enabled = false;
}

bool stepper_update(StepperDriver* d, unsigned long now_us) {
    if (!d->enabled) return false;
    if (now_us - d->last_us < d->interval_us) return false;

    d->last_us = now_us;
    digitalWrite(d->step_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(d->step_pin, LOW);

    if (d->direction) d->step_count++;
    else              d->step_count--;

    return true;
}

void stepper_reset_count(StepperDriver* d) {
    d->step_count = 0;
}
