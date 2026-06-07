#pragma once
#include <stdint.h>

// DWT (Data Watchpoint and Trace) tabanli mikrosaniye sayaci baslatmasi
void hal_time_init(void);

// Arduino micros() fonksiyonunun STM32 karsiligi
uint32_t hal_micros(void);

// Arduino delayMicroseconds() fonksiyonunun STM32 karsiligi
void hal_delay_us(uint32_t us);

// Arduino millis() karsiligi (HAL_GetTick saricisi)
uint32_t hal_millis(void);
