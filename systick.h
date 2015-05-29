/*
 * systick.h - systick routines
 */

#ifndef __systick__
#define __systick__

#include "stm32f0xx.h"

void systick_init(void);
void systick_delayms(uint32_t ms);
uint8_t systick_get_button(void);

#endif
