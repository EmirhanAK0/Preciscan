#include "hal_encoder.h"
#include "stm32f4xx_hal.h"
#include "uart_comm.h"
#include "hal_time.h"

extern I2C_HandleTypeDef hi2c3;

#define AS5600_ADDR        (0x36 << 1)
#define AS5600_RAW_ANGLE_H 0x0C
#define I2C_TIMEOUT_MS     20
#define I2C_MAX_RETRIES    3
#define I2C_RECOVERY_INTERVAL_MS 500

static uint16_t last_raw = 0;
static int32_t enc_counts = 0;
static bool encoder_ok = false;
static uint8_t consecutive_fails = 0;

/* I2C bus recovery: reset the peripheral when bus gets stuck */
static void i2c3_bus_recovery(void) {
    /* Software reset of I2C peripheral */
    __HAL_I2C_DISABLE(&hi2c3);

    /* Toggle SCL manually to unstick a slave holding SDA low */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = GPIO_PIN_8;          /* PA8 = I2C3_SCL */
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    for (int i = 0; i < 16; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
        for (volatile int d = 0; d < 100; d++) {}
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
        for (volatile int d = 0; d < 100; d++) {}
    }

    /* Reconfigure pin back to AF for I2C */
    gpio.Mode      = GPIO_MODE_AF_OD;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* Also reconfigure SDA pin with pull-up */
    gpio.Pin = GPIO_PIN_9;            /* PC9 = I2C3_SDA */
    HAL_GPIO_Init(GPIOC, &gpio);

    /* Re-init the I2C peripheral */
    HAL_I2C_Init(&hi2c3);

    consecutive_fails = 0;
}

static bool as5600_read_raw(uint16_t* raw) {
    uint8_t data[2];

    for (uint8_t retry = 0; retry < I2C_MAX_RETRIES; retry++) {
        /* Check if I2C bus is busy/stuck before attempting read */
        if (hi2c3.State != HAL_I2C_STATE_READY) {
            __HAL_I2C_DISABLE(&hi2c3);
            __HAL_I2C_ENABLE(&hi2c3);
        }

        HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
            &hi2c3,
            AS5600_ADDR,
            AS5600_RAW_ANGLE_H,
            I2C_MEMADD_SIZE_8BIT,
            data,
            2,
            I2C_TIMEOUT_MS
        );

        if (status == HAL_OK) {
            *raw = ((uint16_t)data[0] << 8) | data[1];
            *raw &= 0x0FFF;
            consecutive_fails = 0;
            return true;
        }
    }

    /* All retries failed */
    consecutive_fails++;

    /* After several consecutive failures, try bus recovery */
    if (consecutive_fails >= 5) {
        i2c3_bus_recovery();
    }

    return false;
}

void encoder_init(void) {
    /* Try bus recovery first in case I2C is stuck from a previous session */
    i2c3_bus_recovery();

    uint16_t raw;
    if (as5600_read_raw(&raw)) {
        last_raw = raw;
        enc_counts = 0;
        encoder_ok = true;
    } else {
        last_raw = 0;
        enc_counts = 0;
        encoder_ok = false;
        uart_comm_println("STATUS:ERR:ENC_INIT_FAIL");
    }
}

void encoder_update(void) {
    uint16_t raw;
    if (!as5600_read_raw(&raw)) {
        static uint32_t last_err_ms = 0;
        if (hal_millis() - last_err_ms > 1000) {
            uart_comm_println("STATUS:ERR:ENC_I2C");
            last_err_ms = hal_millis();
        }
        encoder_ok = false;
        return;
    }

    if (!encoder_ok) {
        uart_comm_println("STATUS:INFO:ENC_RECOVERED");
    }

    int16_t diff = (int16_t)raw - (int16_t)last_raw;

    // Wraparound handler
    if (diff > 2048) {
        diff -= 4096;
    } else if (diff < -2048) {
        diff += 4096;
    }

    // You may invert this += or -= depending on physical rotation vs desired angle direction
    // For now we add diff directly.
    enc_counts += diff;
    last_raw = raw;
    encoder_ok = true;
}

float encoder_get_angle(void) {
    // 4096 counts = 360 degrees
    return (float)enc_counts * (360.0f / 4096.0f);
}

void encoder_reset(void) {
    enc_counts = 0;
}
