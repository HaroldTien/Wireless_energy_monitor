/*
 * timer0.c
 *
 * Created: 19/09/2025 5:54:34 PM
 *  Author: thar970
 */ 

#include "timer0.h"
#include "led.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#include "timer0.h"
#include <avr/io.h>

void timer0_init(void) {
	// Configure Timer0 for Normal mode
	TCCR0A = 0x00;
	// Keep the timer stopped initially (no clock source)
	TCCR0B = 0x00;
	//ensure the timer start from zero
	TCNT0 = 0x00;
}

void timer0_start(void) {
	// Reset the timer count register
	TCNT0 = 0;
	// Start the timer with a prescaler of 64
	TCCR0B = (1 << CS01) | (1 << CS00);
}

uint8_t timer0_stop(void) {
	// Stop the timer by selecting no clock source
	TCCR0B = 0x00;
	// Return the final count
	return TCNT0;
}