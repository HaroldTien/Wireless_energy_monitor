/*
 * Lab5.c
 *
 * Created: 19/09/2025 5:51:29 PM
 * Author : thar970
 */ 

#define F_CPU 2000000UL // System clock frequency is 2 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "uart.h"
#include "timer0.h"
void interrupt_init(void);

// Volatile variables to safely share data between the ISR and main loop
volatile uint8_t timer_count = 0;
volatile uint8_t measurement_ready = 0;

// Create and setup the stream for printf redirection
static FILE usart_stdout = FDEV_SETUP_STREAM(uart_printf, NULL, _FDEV_SETUP_WRITE);




int main(void){
	
	
    // --- INITIALIZATION ---
    uart_init(9600);
    stdout = &usart_stdout; // Redirect stdout to go to our UART
    timer0_init();
    interrupt_init();
    sei(); // Enable global interrupts

	 
    while (1){
		if (measurement_ready) {
			uint16_t count = timer_count;
			uint32_t half_us = (uint32_t)count * 32u;     // prescaler=64 ? 32 µs/tick
			uint32_t f_x100 = (half_us ? (50000000u / half_us) : 0u);  // f = 50,000,000 / Thalf_us
			uint32_t f_int  = f_x100 / 100u;
			uint32_t f_frac = f_x100 % 100u;

			char line[64];
			
			sprintf(line, "count=%u, Thalf=%lu us, f=%lu.%02lu Hz\r\n",
			(unsigned)count,
			(unsigned long)half_us,
			(unsigned long)f_int,
			(unsigned long)f_frac);

			uart_transmit_array(line);
			measurement_ready = 0;
		}
    }
	return 0;
}

void interrupt_init(void){
	// Configure INT0 to trigger on a rising edge initially
	EICRA |= (1 << ISC01) | (1 << ISC00);
	// Enable the INT0 external interrupt
	EIMSK |= (1 << INT0);
}

// --- INTERRUPT SERVICE ROUTINE for INT0 ---
ISR(INT0_vect) {
	// Check if the interrupt was triggered by a rising edge (ISC00 is 1)
	if (EICRA & (1 << ISC00)) {
		timer0_start();
		// Reconfigure interrupt to trigger on the next falling edge
		EICRA &= ~(1 << ISC00);
	}
	// Otherwise, the interrupt was triggered by a falling edge
	else {
		timer_count = timer0_stop();
		measurement_ready = 1; // Signal the main loop that data is ready
		// Reconfigure interrupt back to trigger on a rising edge for the next cycle
		EICRA |= (1 << ISC00);
	}
}