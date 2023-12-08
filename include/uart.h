/**********************************************************
 * 
 *  uart.h
 * 
 * 
 *  DESCRIPTION:
 *      UART driver interface
 *
 */

#include "generic.h"

void uart_init(void);
void uart_send(char c);
uint8_t uart_recv(void);
void uart_send_string(char *str);
boolean uart_is_init(void);
