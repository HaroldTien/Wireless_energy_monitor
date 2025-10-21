#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <stdint.h>

// Function declarations
void usart_init(uint8_t prescaler);
void usart_transmit(uint8_t data);
void usart_transmit_string(const char* str);
void usart_transmit_number(uint16_t number);
void usart_transmit_float(float value, uint8_t decimals);
void usart_send_power_data(void);

#endif // UART_H
