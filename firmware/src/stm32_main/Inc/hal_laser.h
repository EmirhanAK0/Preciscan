#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "bsp_pins.h"

typedef struct {
    // Pin port ve numarasi BSP_LASER_PORT olarak kullaniliyor, struct icinde tutmaya gerek yok (veya tutulabilir).
    bool active;
    uint32_t trig_us;
    uint32_t pulse_us;
} LaserTrigger;

// Init eder (pin'i kapatir). pin argumani kaldirildi, bsp_pins.h kullaniliyor.
void laser_init(LaserTrigger* lt, uint32_t pulse_us);

// Pulse baslat
void laser_fire(LaserTrigger* lt);

// Hemen kapat
void laser_off(LaserTrigger* lt);

// Otomatik sondurme
void laser_update(LaserTrigger* lt, uint32_t now_us);

bool laser_is_active(const LaserTrigger* lt);
