#pragma once
#include <stdint.h>
#include <stdbool.h>

void uart_comm_init(void);

// UART'tan gelen karakterleri buffer'a yazar (Interrupt icinde cagirilir)
void uart_comm_rx_callback(uint8_t rx_char);

// TX interrupt tamamlandiginda cagirilir — bir sonraki byte'i gonderir
void uart_comm_tx_callback(void);

// Satir sonu (\n) gelene kadar bekleyen veriyi okur
// Donus degeri: Eger tam bir satir okunduysa true doner.
bool uart_comm_read_line(char* out_buf, uint16_t max_len);

// Seri porttan string gonderir (non-blocking, TX ring buffer kullanir)
void uart_comm_print(const char* str);
void uart_comm_println(const char* str);
void uart_comm_print_float(const char* prefix, float val, int decimals);
