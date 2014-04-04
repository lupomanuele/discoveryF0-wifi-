#ifndef __DYSP_USART_WIFI__
#define __DYSP_USART_WIFI__

#include <stm32f0xx_usart.h>

#define MAX_STRLEN 512 // this is the maximum string length of our string in characters
#define EXTI0_1_IRQn 5
#define WIFI_USART				USART1
#define WIFI_USART_IRQn			USART1_IRQn
#define WIFI_USART_IRQHandler   USART1_IRQHandler

volatile int result_index;
void EVAL_WIFI_UART_init(uint32_t baudrate);
void EVAL_WIFI_UART_puts(USART_TypeDef* USARTx, char *s, int count);
void EVAL_WIFI_UART_cleanupBuffer2RB(USART_TypeDef* USARTx );
uint8_t EVAL_WIFI_UART_send_and_test(USART_TypeDef* USARTx, char *s, char *c, uint32_t timeout, int size);
void EVAL_WIFI_UART_addBuffer2RB(USART_TypeDef* USARTx, char *s, uint8_t count );
void EVAL_WIFI_UART_IRQHandler(void);
int my_strstr(char *string1 , char *string2);
volatile uint32_t time_var2;
#endif /* __DYSP_USART_WIFI__ */
