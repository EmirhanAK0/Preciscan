// ============================================================
// BSP — Board Support Package (Arduino Mega / Uno Pin Haritasi)
// Donanim pini degisiklikleri YALNIZCA bu dosyada yapilir.
// ============================================================
#pragma once
#include <Arduino.h>

// ---- Motor Pin Cifleri ----
struct MotorPins {
    uint8_t step;
    uint8_t dir;
};

static const MotorPins BSP_LIN_MOTOR = { 11, 12 };
static const MotorPins BSP_ROT_MOTOR = {  9, 10 };
static const MotorPins BSP_Z_MOTOR   = {  3,  4 };

// ---- Limit Switch Pinleri ----
struct LimitPins {
    uint8_t home;
    uint8_t safety;
};

static const LimitPins BSP_LIN_LIMITS = { 5, 6 };
static const LimitPins BSP_Z_LIMITS   = { 7, 8 };

// ---- Lazer Tetik Pini ----
static const uint8_t BSP_LASER_PIN = A0;

// ---- Yon Elektrik Seviyeleri ----
static const uint8_t BSP_DIR_FORWARD = LOW;
static const uint8_t BSP_DIR_REVERSE = HIGH;

// ---- Seri Port ----
static const uint32_t BSP_SERIAL_BAUD = 115200;
