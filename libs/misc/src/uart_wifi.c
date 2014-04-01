/**
 * @file dysp_usart_wifi.c
 * @author Manuele Lupo
 * @date 9 Sep 2013
 * @brief This file contains a set of low-level APIs to deal with the SPWF serial console on USART2
 *
 */

#include "uart_wifi.h"
#include "stm32f0xx_dma.h"
#include "stm32f0xx_misc.h"
#include <string.h>

#define ERR_OK          0

volatile uint32_t received_cnt = 0; // this counter is used to determine the string length
volatile char received_string[MAX_STRLEN + 1]; // this will hold the received string

static uint8_t rx_buffer[2];
// This variable is used to identify if we are in a thread context or not.
// to select the best delay approach we should use.
//extern void os_Delay(volatile uint32_t nCount);
/* This function initializes the USART2 peripheral
 *
 * Arguments: baudrate --> the baudrate at which the USART is
 * 						   supposed to operate
 */

void EVAL_WIFI_UART_init(uint32_t baudrate) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	//DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)

	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* Enable USART clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Enable the DMA periph */
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	/* Connect PXx to USARTx_Tx */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);

	/* Connect PXx to USARTx_Rx */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

	/* Configure USART Tx and Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#ifdef USE_DMA_UART
	/* DMA Configuration -------------------------------------------------------*/
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	/* Configure the USART to receive */
	/* DMA channel Rx of USART Configuration */
	//DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013824;
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) CharBuffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	//DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	/* Configure the USART to send */
	/* DMA channel Tx of USART Configuration */
	// DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013828;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	//DMA_Init(DMA1_Channel2, &DMA_InitStructure);

	/* Enable the DMA channel */
	// DMA_Cmd(DMA1_Channel3, ENABLE);
#else
	/* Enable USART2 Rx request */
	USART_ITConfig(WIFI_USART, USART_IT_TXE, DISABLE);
	USART_ITConfig(WIFI_USART, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = WIFI_USART_IRQn; // we want to configure the USART2 interrupts
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// the USART2 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure);	// the properties are passed to the NVIC_Init function which takes care of the low level stuff

#endif
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(WIFI_USART, &USART_InitStructure);

#ifdef USE_DMA_UART
	/* Enable the USART Rx DMA request */
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	/* Enable the USART Tx DMA request */
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
#endif

	// finally this enables the complete USART1 peripheral
	USART_Cmd(WIFI_USART, ENABLE);

}

/* This function is used to transmit a string of characters via
 * the USART specified in USARTx.
 *
 * It takes two arguments: USARTx --> can be any of the USARTs e.g. USART2, USART2 etc.
 * 						   (volatile) char *s is the string you want to send
 *
 * Note: The string has to be passed to the function as a pointer because
 * 		 the compiler doesn't know the 'string' data type. In standard
 * 		 C a string is just an array of characters
 *
 * Note 2: At the moment it takes a volatile char because the received_string variable
 * 		   declared as volatile char --> otherwise the compiler will spit out warnings
 * */
void EVAL_WIFI_UART_puts(USART_TypeDef* USARTx, char *s, int count) {

	//USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	int i = 0;

	while (i < count) {
		USART_SendData(USARTx, (uint8_t) s[i++]);
		//USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
	}

}

void EVAL_WIFI_UART_clean(USART_TypeDef* USARTx) {
	char u8t_resetSequence[] = { 0x1B, '[', '2', 'J', 0x1B, '[', '0', '0', 'H',
			'\0' };
	EVAL_WIFI_UART_puts(USARTx, u8t_resetSequence, strlen(u8t_resetSequence));
}

void EVAL_WIFI_UART_addBuffer2RB(USART_TypeDef* USARTx, char *s, uint8_t count) {
	uint8_t i = 0;
	uint32_t index = received_cnt;

	if (index + count >= MAX_STRLEN) {
		EVAL_WIFI_UART_cleanupBuffer2RB(USARTx);
		return;
	}

	for (i = 0; i < count; i++) {
		received_string[i + index] = s[i];
		received_cnt++;
	}

	//received_string[received_cnt] = '\0';
}

void EVAL_WIFI_UART_cleanupBuffer2RB(USART_TypeDef* USARTx) {
	memset((void *) received_string, 0, received_cnt);
	received_string[received_cnt] = '\0';
	received_cnt = 0;
}

uint16_t EVAL_WIFI_UART_readRB(USART_TypeDef* USARTx, uint8_t *buff) {
	uint16_t i = 0;
	if (buff != NULL && received_cnt > 0)
		while (received_string[i] != '\0' && i < received_cnt) {
			buff[i] = received_string[i];
			i++;
		}

	return i;
}

uint8_t EVAL_WIFI_UART_send_and_test(USART_TypeDef* USARTx, char *s, char *c,
		uint32_t timeout, int size) {
	uint8_t ret = ERR_OK + 1;
	result_index = -1;
	int retry = 10;

	if (s != NULL) {
		EVAL_WIFI_UART_cleanupBuffer2RB(USARTx);
		EVAL_WIFI_UART_puts(USARTx, s, size);
	}
	ret = 0;

	if (!c) {
		return ERR_OK;
	}

	WAIT: retry--;
	if (retry == 0)
		return ret;
	os_Delay(timeout);

	if (received_cnt > 0) {
		result_index = my_strstr((char*) received_string, (char*) c);
		if (result_index >= 0) {
			ret = ERR_OK;
		} else
			goto WAIT;
	}

	return ret;
}

void USART1_IRQHandler(void) {
	static int tx_index = 0;

	if (USART_GetITStatus(WIFI_USART, USART_IT_RXNE) != RESET) // Received characters modify string
			{
		/* Read one byte from the receive data register */
		//STM_EVAL_LEDToggle(LED4);
		rx_buffer[0] = USART_ReceiveData(WIFI_USART);
		if (rx_buffer[0] != '\n' && rx_buffer[0] != '\r')
			EVAL_WIFI_UART_addBuffer2RB(WIFI_USART, (char*) rx_buffer, 1);
	}

	if (USART_GetITStatus(WIFI_USART, USART_IT_TXE) != RESET) // Transmit the string in a loop
			{
		tx_index++;

		USART_ITConfig(WIFI_USART, USART_IT_TXE, DISABLE); // Just to be sure ;-)
	}

}

/*
 * We are using this custom strstr because some times we need to maange string
 * with multiple '\0' and the std strstr doesn't work properly.
 */
int my_strstr(char *string1, char *string2) {
#if 0
	int offset = 0;
	int c = 0;
	int last, last_1;
	char* tmp = strstr(string1, string2);
	if (tmp != NULL)
	offset = (tmp - string1);

	return offset;
#else
	char tmp[256];
	char *tt = NULL;
	int loop = 0;
	memcpy(tmp, string1, 256);
	// Search \0
	while (loop < 256) {
		if (tmp[loop] == '\0')
			tmp[loop] = 'X';

		tt = strstr(tmp, string2);
		if (tt) {
			return (tt - tmp);
		}

		loop++;
	}

	return 0;
#endif
}

