#pragma once
#include <stdint.h>
#include <stdbool.h>

void encoder_init(void);
void encoder_update(void);
float encoder_get_angle(void);
int32_t encoder_get_counts(void);
void encoder_reset(void);
