/*
 * target.c
 *
 *  Created on: Apr 4, 2014
 *      Author: manuele
 */
#if 1
#include "target.h"

/* ----- SysTick definitions ----------------------------------------------- */


static __IO uint32_t uwTimingDelay;
#define BLINK_PORT      GPIOC
#define BLINK_PIN       9
#define BLINK_RCC_BIT   RCC_AHBPeriph_GPIOC
#define BLINK_TICKS     SYSTICK_FREQUENCY_HZ/2

/* ----- LED definitions --------------------------------------------------- */

/* STM32F0DISCOVERY definitions (the GREEN LED) */
/* Adjust them for your own board. */

#define BLINK_PORT      GPIOC
#define BLINK_PIN       9
#define BLINK_RCC_BIT   RCC_AHBPeriph_GPIOC

#define BLINK_TICKS     SYSTICK_FREQUENCY_HZ/2

/**
 * @brief  Inserts a delay time.
 * @param  nTime: specifies the delay time length, in SysTick ticks.
 * @retval None
 */
void os_Delay(__IO uint32_t nTime) {
	// Convert ms to ticks number
	//uint32_t p = (SystemCoreClock / 1000)*nTime;
	uwTimingDelay = nTime;

	while (uwTimingDelay != 0)
		;
}

/**
 * @brief  Decrements the TimingDelay variable.
 * @param  None
 * @retval None
 */
static void TimingDelay_Decrement(void) {
	if (uwTimingDelay != 0x00) {
		uwTimingDelay--;
	}
}

// ----------------------------------------------------------------------------

/**
 * @brief  This function is the SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void) {
	TimingDelay_Decrement();

	uwTimingPerf++;
}

// ----------------------------------------------------------------------------

/**
 * Main Platform Init Routine
 */
void main_target_init() {
	/*
	 * At this stage the microcontroller clock setting is already configured,
	 * this is done through SystemInit() function which is called from startup
	 * file (startup_cm.c) before to branch to application main.
	 * To reconfigure the default setting of SystemInit() function, refer to
	 * system_stm32f0xx.c file
	 */

	/* Use SysTick as reference for the timer */
	SysTick_Config(SystemCoreClock / SYSTICK_FREQUENCY_HZ);

	/* GPIO Periph clock enable */
	RCC_AHBPeriphClockCmd(BLINK_RCC_BIT, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure pin in output push/pull mode */
	GPIO_InitStructure.GPIO_Pin = (1 << BLINK_PIN);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BLINK_PORT, &GPIO_InitStructure);

}

/**
 * Api to manage green led
 * @param status 0==led OFF. Else on.
 */
void led_set(uint8_t status) {
	if (status == 0)
		GPIO_ResetBits(BLINK_PORT, (1 << BLINK_PIN));
	else
		GPIO_ResetBits(BLINK_PORT, (1 << BLINK_PIN));
}

#endif
