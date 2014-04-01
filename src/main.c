#include "stm32f0xx.h"
#include <stdio.h>
#include "spwf_wifi.h"

/*
 * STM32F0 led blink sample (retargetted to semihosting).
 *
 * In debug configurations, demonstrate how to print a greeting message
 * on the standard output. In release configurations the message is
 * simply discarded. By default the trace messages are forwarded to
 * semihosting output, but can be completely suppressed by changing
 * the definitions in misc/include/trace_impl.h.
 *
 * Then demonstrates how to blink a led with 1Hz, using a
 * continuous loop and SysTick delays.
 *
 * On DEBUG, the uptime in seconds is also displayed on the standard output.
 *
 * The external clock frequency is specified as a preprocessor definition
 * passed to the compiler via a command line option (see the 'C/C++ General' ->
 * 'Paths and Symbols' -> the 'Symbols' tab, if you want to change it).
 * The value selected during project creation was HSE_VALUE=8000000.
 *
 * Note: The default clock settings take the user defined HSE_VALUE and try
 * to reach the maximum possible system clock. For the default 8MHz input
 * the result is guaranteed, but for other values it might not be possible,
 * so please adjust the PLL settings in libs/CMSIS/src/system_stm32f0xx.c
 *
 * The build does not use startup files, and on Release it does not even use
 * any standard library function (on Debug the printf() brings lots of
 * functions; removing it should also use no other standard lib functions).
 *
 * If the application requires special initialisation code present
 * in some other libraries (for example librdimon.a, for semihosting),
 * define USE_STARTUP_FILES and uncheck the corresponding option in the
 * linker configuration.
 *
 */

// ----------------------------------------------------------------------------
void os_Delay(__IO uint32_t nTime);
void GPIO_Configuration(void);
static void
TimingDelay_Decrement(void);

void
SysTick_Handler(void);

/* ----- SysTick definitions ----------------------------------------------- */

#define SYSTICK_FREQUENCY_HZ       1000

/* ----- LED definitions --------------------------------------------------- */

/* STM32F0DISCOVERY definitions (the GREEN LED) */
/* Adjust them for your own board. */

#define BLINK_PORT      GPIOC
#define BLINK_PIN       9
#define BLINK_RCC_BIT   RCC_AHBPeriph_GPIOC

#define BLINK_TICKS     SYSTICK_FREQUENCY_HZ/2

// ----------------------------------------------------------------------------

int main(void) {
#if defined(DEBUG)
	/*
	 * Send a greeting to the standard output (the semihosting debug channel
	 * on Debug, ignored on Release).
	 */
	GPIO_Configuration();
	//printf("Hello ARM World!\n");
#endif
	struct wifi_spwf_conf cfg;
	cfg.baudrate = 115200;
	cfg.mode = sta;
	sprintf(cfg.ssid, "IoT");
	sprintf(cfg.wpa_key, "IoT1Time2Go");

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

	int seconds = 0;
	spwf_configure(&cfg, 1);

	/* Infinite loop */
	while (1) {
		/* Assume the LED is active low */

		/* Turn on led by setting the pin low */
		GPIO_ResetBits(BLINK_PORT, (1 << BLINK_PIN));

		os_Delay(BLINK_TICKS);

		/* Turn off led by setting the pin high */
		GPIO_SetBits(BLINK_PORT, (1 << BLINK_PIN));

		os_Delay(BLINK_TICKS);

		++seconds;

#if defined(DEBUG)
		/*
		 * Count seconds on the debug channel.
		 */
		//printf("Second %d\n", seconds);
#endif
	}
}

// ----------------------------------------------------------------------------

static __IO uint32_t uwTimingDelay;

/**
 * @brief  Inserts a delay time.
 * @param  nTime: specifies the delay time length, in SysTick ticks.
 * @retval None
 */
void os_Delay(__IO uint32_t nTime) {
	uwTimingDelay = nTime;

	while (uwTimingDelay != 0)
		;
}

/**
 * @brief  Decrements the TimingDelay variable.
 * @param  None
 * @retval None
 */
void TimingDelay_Decrement(void) {
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
}

/**
 * @brief  Configures the different GPIO ports.
 * @param  None
 * @retval : None
 */
void GPIO_Configuration(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable GPIOC clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure WiFi_Connect as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure LEDs */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Init User/Wakeup button B1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Connect EXTI0 Line to PA0 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	/* Configure EXTI Line0 to generate an interrupt on falling edge */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	/* Enable and set EXTI0 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
// ----------------------------------------------------------------------------

