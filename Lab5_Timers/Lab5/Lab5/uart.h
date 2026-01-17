
/*
 * uart.h
 *
 * Created: 9/20/2025 2:48:51 AM
 *  Author: harol
 */ 
#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Initializes the UART peripheral.
 * @param baud_rate The desired baud rate (e.g., 9600).
 */
void uart_init(uint32_t baud_rate);

/**
 * @brief Transmits a single byte over UART.
 * @param byte The byte to transmit.
 */
void uart_transmit_byte(char byte);

/**
 * @brief Transmits a null-terminated string over UART.
 * @param msg Pointer to the string to transmit.
 */
void uart_transmit_array(char* msg);

/**
 * @brief Wrapper function to be used by FDEV_SETUP_STREAM for printf.
 * @param var The character to be transmitted.
 * @param stream Pointer to the FILE stream.
 * @return Returns 0.
 */
int uart_printf(char var, FILE *stream);

#endif /* UART_H_ */