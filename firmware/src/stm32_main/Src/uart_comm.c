#include "uart_comm.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------
 * RX ring buffer — interrupt ile beslenir (1 byte / interrupt)
 * --------------------------------------------------------------- */
#define RX_BUF_SIZE 256

extern UART_HandleTypeDef huart2;

static uint8_t  rx_ring_buf[RX_BUF_SIZE];
static uint16_t rx_head = 0;
static uint16_t rx_tail = 0;

/* ---------------------------------------------------------------
 * TX ring buffer — non-blocking gonderi icin
 *
 * Neden gerekli?
 *   HAL_UART_Transmit() ile HAL_MAX_DELAY kullanmak ana
 *   donguyu bloke eder. Bu durum motor step zamanlama hassasiyetini
 *   bozar (115200 baud'da 7 byte ~0.6 ms demektir).
 *   TX ring buffer + IT ile gonderim, ana donguyu serbest birakir.
 * --------------------------------------------------------------- */
#define TX_BUF_SIZE 512

static uint8_t  tx_ring_buf[TX_BUF_SIZE];
static uint16_t tx_head = 0;   /* yazilacak bir sonraki konum */
static uint16_t tx_tail = 0;   /* okunacak bir sonraki konum */
static volatile uint8_t tx_busy = 0; /* IT transfer devam ediyor mu? */

/* TX ring buffer'a byte yaz (IRQ disabled degil — kritik bolum kisa) */
static void tx_push(uint8_t byte)
{
    uint16_t next = (tx_head + 1) % TX_BUF_SIZE;
    if (next == tx_tail) return; /* buffer dolu — byte atla */
    tx_ring_buf[tx_head] = byte;
    tx_head = next;
}

/* TX IT callback — bir sonraki byte'i gonder */
void uart_comm_tx_callback(void)
{
    if (tx_tail == tx_head) {
        tx_busy = 0;
        return;
    }
    uint16_t send_tail = tx_tail;
    tx_tail = (tx_tail + 1) % TX_BUF_SIZE;
    HAL_UART_Transmit_IT(&huart2, &tx_ring_buf[send_tail], 1);
}

/* ---------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------- */

void uart_comm_init(void) {
    rx_head = 0;
    rx_tail = 0;
    tx_head = 0;
    tx_tail = 0;
    tx_busy = 0;
}

void uart_comm_rx_callback(uint8_t rx_char) {
    uint16_t next_head = (rx_head + 1) % RX_BUF_SIZE;
    if (next_head != rx_tail) {
        rx_ring_buf[rx_head] = rx_char;
        rx_head = next_head;
    }
}

bool uart_comm_read_line(char* out_buf, uint16_t max_len) {
    if (rx_head == rx_tail) return false;

    /* \r veya \n karakteri var mi kontrol et */
    uint16_t temp_tail = rx_tail;
    bool found_nl = false;
    while (temp_tail != rx_head) {
        if (rx_ring_buf[temp_tail] == '\n' || rx_ring_buf[temp_tail] == '\r') {
            found_nl = true;
            break;
        }
        temp_tail = (temp_tail + 1) % RX_BUF_SIZE;
    }
    if (!found_nl) return false;

    /* Satiri kopyala */
    uint16_t i = 0;
    while (rx_tail != rx_head && i < max_len - 1) {
        char c = (char)rx_ring_buf[rx_tail];
        rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
        if (c == '\n' || c == '\r') {
            /* Arka arkaya gelen \r\n ikililerini temizle */
            while (rx_tail != rx_head &&
                   (rx_ring_buf[rx_tail] == '\n' || rx_ring_buf[rx_tail] == '\r')) {
                rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
            }
            break;
        }
        out_buf[i++] = c;
    }
    out_buf[i] = '\0';
    return true;
}

void uart_comm_print(const char* str) {
    /* TX ring buffer'a yaz, sonra gerekirse transfer baslat */
    while (*str) {
        tx_push((uint8_t)*str++);
    }

    /* Transfer baslatilmamissa ilk byte'i IT ile gonder */
    __disable_irq();
    if (!tx_busy && tx_tail != tx_head) {
        tx_busy = 1;
        uint16_t send_tail = tx_tail;
        tx_tail = (tx_tail + 1) % TX_BUF_SIZE;
        __enable_irq();
        HAL_UART_Transmit_IT(&huart2, &tx_ring_buf[send_tail], 1);
    } else {
        __enable_irq();
    }
}

void uart_comm_println(const char* str) {
    uart_comm_print(str);
    uart_comm_print("\r\n");
}

void uart_comm_print_float(const char* prefix, float val, int decimals) {
    char buf[64];

    /* Negatif deger icin isaret ayri ele alinir,
       sonra tamsayi ve ondalik kisim ayri hesaplanir.
       Bu yontem, platform-agnostic ve NaN-safe'dir. */
    char sign = ' ';
    if (val < 0.0f) {
        sign  = '-';
        val   = -val;
    }

    int i_part = (int)val;
    int f_part;

    if (decimals == 1) {
        f_part = (int)((val - (float)i_part) * 10.0f + 0.5f);
        if (f_part >= 10) { i_part++; f_part = 0; }
        if (sign == '-')
            snprintf(buf, sizeof(buf), "%s-%d.%d", prefix, i_part, f_part);
        else
            snprintf(buf, sizeof(buf), "%s%d.%d",  prefix, i_part, f_part);
    } else if (decimals == 2) {
        f_part = (int)((val - (float)i_part) * 100.0f + 0.5f);
        if (f_part >= 100) { i_part++; f_part = 0; }
        if (sign == '-')
            snprintf(buf, sizeof(buf), "%s-%d.%02d", prefix, i_part, f_part);
        else
            snprintf(buf, sizeof(buf), "%s%d.%02d",  prefix, i_part, f_part);
    } else {
        if (sign == '-')
            snprintf(buf, sizeof(buf), "%s-%d", prefix, i_part);
        else
            snprintf(buf, sizeof(buf), "%s%d",  prefix, i_part);
    }

    uart_comm_println(buf);
}
