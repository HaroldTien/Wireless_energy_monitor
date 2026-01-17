/*
 * uart.h
 *
 * Created: 15/09/2025 1:23:22 PM
 *  Author: thar970
 */ 


#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <stdint.h>

void uart_init(uint32_t baud_rate);
void uart_transmit(uint8_t data);
void uart_transmit_string(const char *str);

#endif
