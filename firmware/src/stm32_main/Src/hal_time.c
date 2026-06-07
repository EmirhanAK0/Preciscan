#include "hal_time.h"
#include "stm32f4xx_hal.h"

static uint32_t cyclesPerUs = 84; 

void hal_time_init(void) {
    cyclesPerUs = HAL_RCC_GetHCLKFreq() / 1000000UL;
}

uint32_t hal_micros(void) {
    uint32_t ms = HAL_GetTick();
    uint32_t st_val = SysTick->VAL;
    uint32_t st_load = SysTick->LOAD;
    
    // SysTick geriye sayan bir counter'dir. (LOAD - VAL) bize gecen cycle'i verir.
    uint32_t us_part = (st_load - st_val) / cyclesPerUs;
    
    return (ms * 1000) + us_part;
}

void hal_delay_us(uint32_t us) {
    uint32_t start = hal_micros();
    while (hal_micros() - start < us) {
        // Bekle
    }
}

uint32_t hal_millis(void) {
    return HAL_GetTick();
}
