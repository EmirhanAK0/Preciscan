// ============================================================
// HAL — Lazer Tetik Kontrolu
// ============================================================
#pragma once
#include <Arduino.h>

struct LaserTrigger {
    uint8_t pin;
    bool active;
    unsigned long trig_us;
    unsigned long pulse_us;
};

void laser_init(LaserTrigger* lt, uint8_t pin, unsigned long pulse_us);
void laser_fire(LaserTrigger* lt);               // Pulse baslat
void laser_off(LaserTrigger* lt);                 // Hemen kapat
void laser_update(LaserTrigger* lt, unsigned long now_us);  // Otomatik sondurme
bool laser_is_active(const LaserTrigger* lt);
