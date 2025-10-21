#include "uart.h"
#include "powercalc.h"
#include <stdint.h>

// UART Initialization
void usart_init(uint8_t prescaler)
{
    // Set baud rate
    UBRR0H = (uint8_t)(prescaler >> 8);
    UBRR0L = (uint8_t)(prescaler);
    
    // Enable transmitter only (TX only mode)
    UCSR0B = (1 << TXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Transmit single byte
void usart_transmit(uint8_t data)
{
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    
    // Put data into buffer, sends the data
    UDR0 = data;
}

// Transmit null-terminated string
void usart_transmit_string(const char* str)
{
    while (*str != '\0') {
        usart_transmit(*str);
        str++;
    }
}

// Transmit number as string
void usart_transmit_number(uint16_t number)
{
    char buffer[6]; // Max 5 digits + null terminator
    uint8_t i = 0;
    
    if (number == 0) {
        usart_transmit('0');
        return;
    }
    
    // Convert number to string (reverse order)
    while (number > 0) {
        buffer[i++] = '0' + (number % 10);
        number /= 10;
    }
    
    // Transmit in correct order
    for (uint8_t j = i; j > 0; j--) {
        usart_transmit(buffer[j-1]);
    }
}

// Transmit float with specified decimal places
void usart_transmit_float(float value, uint8_t decimals)
{
    // Handle negative numbers
    if (value < 0) {
        usart_transmit('-');
        value = -value;
    }
    
    // Extract integer part
    uint32_t integer_part = (uint32_t)value;
    usart_transmit_number(integer_part);
    
    if (decimals > 0) {
        usart_transmit('.');
        
        // Extract decimal part
        float decimal_part = value - integer_part;
        for (uint8_t i = 0; i < decimals; i++) {
            decimal_part *= 10;
            uint8_t digit = (uint8_t)decimal_part;
            usart_transmit('0' + digit);
            decimal_part -= digit;
        }
    }
}

// Send power monitoring data via UART
void usart_send_power_data(void)
{
    // Check if display data is ready
    if (is_display_data_ready()) {
        // Send actual power data
        usart_transmit_string("Average Power = ");
        usart_transmit_float(get_display_power(), 1);
        usart_transmit_string(" W\r\n");
        
        usart_transmit_string("RMS Voltage = ");
        usart_transmit_float(get_display_voltage(), 1);
        usart_transmit_string(" V\r\n");
        
        usart_transmit_string("Peak Current = ");
        usart_transmit_float(get_display_current(), 1);
        usart_transmit_string(" mA\r\n");
        
        usart_transmit_string("---\r\n");
    } else {
        // Send "no signal" status message
        usart_transmit_string("No Signal Detected\r\n");
        usart_transmit_string("Waiting for INT0 trigger...\r\n");
        usart_transmit_string("---\r\n");
    }
}
