#include "hal_laser.h"
#include "stm32f4xx_hal.h"
#include "hal_time.h"

void laser_init(LaserTrigger* lt, uint32_t pulse_us) {
    lt->active   = false;
    lt->trig_us  = 0;
    lt->pulse_us = pulse_us;
    laser_off(lt);
}

void laser_fire(LaserTrigger* lt) {
    lt->active  = true;
    lt->trig_us = hal_micros();
    HAL_GPIO_WritePin(BSP_LASER_PORT, BSP_LASER_PIN, GPIO_PIN_SET);
}

void laser_off(LaserTrigger* lt) {
    lt->active = false;
    HAL_GPIO_WritePin(BSP_LASER_PORT, BSP_LASER_PIN, GPIO_PIN_RESET);
}

void laser_update(LaserTrigger* lt, uint32_t now_us) {
    if (lt->active) {
        if (now_us - lt->trig_us > lt->pulse_us) {
            laser_off(lt);
        }
    }
}

bool laser_is_active(const LaserTrigger* lt) {
    return lt->active;
}
