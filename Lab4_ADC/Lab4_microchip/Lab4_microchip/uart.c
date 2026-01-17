	/*
 * uart.c
 *
 * Created: 15/09/2025 1:22:55 PM
 *  Author: thar970
 */ 
#include "uart.h"

#define F_CPU 2000000UL  // 2 MHz system clock

void uart_init(uint32_t baud_rate) {
	// Baud rate formula: UBRR0 = (F_CPU / (16 * baud_rate)) - 1
	uint16_t ubrr_value = (F_CPU / (16UL * baud_rate)) - 1;

	UBRR0H = (uint8_t)(ubrr_value >> 8);
	UBRR0L = (uint8_t)ubrr_value;

	// Enable transmitter
	UCSR0B = (1 << TXEN0);

	// Frame format: 8 data bits, 1 stop bit, no parity
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(uint8_t data) {
	while (!(UCSR0A & (1 << UDRE0)));  // Wait until buffer is empty
	UDR0 = data;
}

void uart_transmit_string(const char *str) {
	while (*str) {
		uart_transmit(*str++);
	}
}
