/*
 * target.h
 *
 *  Created on: Apr 4, 2014
 *      Author: manuele
 */

#ifndef TARGET_H_
#define TARGET_H_

#include "stm32f0xx.h"

// ----------------------------------------------------------------------------
void os_Delay(__IO uint32_t nTime);
void led_set(uint8_t status);
void main_target_init();

// ----------------------------------------------------------------------------
__IO uint32_t uwTimingPerf;

#endif /* TARGET_H_ */
