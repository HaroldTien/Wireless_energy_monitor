/*
 * uart.c
 *
 * Created: 9/20/2025 2:49:09 AM
 *  Author: harol
 */ 

#define F_CPU 2000000UL // System clock frequency is 2 MHz
#include "uart.h"
#include <avr/io.h>
#include <string.h>

void uart_init(uint32_t baud_rate) {
	// Set baud rate using the UBRR0 register
	// The formula is UBRR = F_CPU / (16 * BAUD) - 1
	uint16_t ubrr_value = (F_CPU / (16UL * baud_rate)) - 1;
	UBRR0H = (uint8_t)(ubrr_value >> 8);
	UBRR0L = (uint8_t)ubrr_value;
	
	// Enable transmitter
	UCSR0B = (1 << TXEN0);
	
	// Set frame format: 8 data bits, 1 stop bit (8N1)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit_byte(char byte) {
	// Wait for the transmit buffer to be empty
	while (!(UCSR0A & (1 << UDRE0)));
	// Put data into buffer, sends the data
	UDR0 = byte;
}

void uart_transmit_array(char* msg) {
	for (uint8_t i = 0; i < strlen(msg); i++) {
		uart_transmit_byte(msg[i]);
	}
}

int uart_printf(char var, FILE *stream) {
	uart_transmit_byte(var);
	return 0;
}