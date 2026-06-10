#pragma once
#include <stdint.h>
#include <stdbool.h>

void encoder_init(void);
void encoder_update(void);
float encoder_get_angle(void);
void encoder_reset(void);
