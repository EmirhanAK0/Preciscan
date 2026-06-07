// ============================================================
// BSP — Board Support Package (STM32F446RE Pin Haritasi)
// Donanim pini degisiklikleri YALNIZCA bu dosyada yapilir.
// ============================================================
#pragma once
#include "stm32f4xx_hal.h"

// ---- Motor Pin Cifleri ----
typedef struct {
    GPIO_TypeDef* step_port;
    uint16_t      step_pin;
    GPIO_TypeDef* dir_port;
    uint16_t      dir_pin;
    GPIO_TypeDef* en_port;
    uint16_t      en_pin;
} MotorPins;

// X Ekseni (Lineer)
static const MotorPins BSP_LIN_MOTOR = { GPIOA, GPIO_PIN_10, GPIOB, GPIO_PIN_3, GPIOA, GPIO_PIN_0 };
// C Ekseni (Rotary / Scanner)
static const MotorPins BSP_ROT_MOTOR = { GPIOA, GPIO_PIN_9,  GPIOC, GPIO_PIN_7, GPIOA, GPIO_PIN_4 };
// Z Ekseni
static const MotorPins BSP_Z_MOTOR   = { GPIOB, GPIO_PIN_5,  GPIOB, GPIO_PIN_4, GPIOA, GPIO_PIN_1 };

// ---- Limit Switch Pinleri ----
typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
} LimitPinDef;

typedef struct {
    LimitPinDef home;
    LimitPinDef safety;
} LimitPins;

// X_MIN: PB0, X_MAX: PC1
static const LimitPins BSP_LIN_LIMITS = { {GPIOB, GPIO_PIN_0}, {GPIOC, GPIO_PIN_1} };
// Z_MIN: PA6 (Home), Z_MAX: PC0 (Safety)
static const LimitPins BSP_Z_LIMITS   = { {GPIOA, GPIO_PIN_6}, {GPIOC, GPIO_PIN_0} };

// ---- Lazer Tetik Pini ----
#define BSP_LASER_PORT GPIOC
#define BSP_LASER_PIN  GPIO_PIN_2

// ---- Yon Elektrik Seviyeleri ----
// A4988 / TMC suruculere gore yon belirler (LOW veya HIGH)
#define BSP_DIR_FORWARD GPIO_PIN_RESET
#define BSP_DIR_REVERSE GPIO_PIN_SET
