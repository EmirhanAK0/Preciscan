#include "hal_stepper.h"
#include "hal_time.h"

void stepper_init(StepperDriver* d, MotorPins pins, uint32_t interval_us) {
    d->pins        = pins;
    d->interval_us = interval_us;
    d->last_us     = hal_micros();
    d->enabled     = false;
    d->direction   = true;
    d->step_count  = 0;

    // Pin konfigurasyonlari (GPIO Init) CubeMX tarafindan yapildigi varsayilir.
    // Eger dinamik yapmak gerekirse buraya HAL_GPIO_Init eklenebilir.
    
    stepper_set_dir(d, true);
}

void stepper_set_dir(StepperDriver* d, bool forward) {
    d->direction = forward;
    HAL_GPIO_WritePin(d->pins.dir_port, d->pins.dir_pin, 
                      forward ? BSP_DIR_FORWARD : BSP_DIR_REVERSE);
}

void stepper_set_interval(StepperDriver* d, uint32_t us) {
    d->interval_us = us;
}

void stepper_enable(StepperDriver* d) {
    d->enabled = true;
    d->last_us = hal_micros();
    // TMC sürücüleri için Enable LOW aktif
    HAL_GPIO_WritePin(d->pins.en_port, d->pins.en_pin, GPIO_PIN_RESET);
}

void stepper_disable(StepperDriver* d) {
    d->enabled = false;
    // Motoru serbest bırakmak için HIGH
    HAL_GPIO_WritePin(d->pins.en_port, d->pins.en_pin, GPIO_PIN_SET);
}

bool stepper_update(StepperDriver* d, uint32_t now_us) {
    if (!d->enabled) return false;
    if (now_us - d->last_us < d->interval_us) return false;

    d->last_us = now_us;
    
    // Step pulse olustur
    HAL_GPIO_WritePin(d->pins.step_port, d->pins.step_pin, GPIO_PIN_SET);
    hal_delay_us(5); // 5us bekleme (TMC veya A4988 icin yeterli)
    HAL_GPIO_WritePin(d->pins.step_port, d->pins.step_pin, GPIO_PIN_RESET);

    if (d->direction) d->step_count++;
    else              d->step_count--;

    return true;
}

void stepper_reset_count(StepperDriver* d) {
    d->step_count = 0;
}
