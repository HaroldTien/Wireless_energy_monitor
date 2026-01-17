/*
 * Lab4_preLabMicrochip.c
 *
 * Created: 9/15/2025 9:51:04 AM
 * Author : harol
 */ 

// ATmega328P — blink LED on PB5 at 1 Hz, 50% duty
#define F_CPU 16000000UL        // use 8000000UL if your board is 8 MHz
#include <avr/io.h>
#include <util/delay.h>


int main(void){
	// LED on PB5
	DDRB  |= (1<<PB5);

	// Button on PD2: input with pull-up (button pulls to GND when pressed)
	DDRD  &= ~(1<<PD2);
	PORTD |=  (1<<PD2);

	// start LED off
	PORTB &= ~(1<<PB5);

	for(;;){
		// read PD2; pressed = 0
		uint8_t pressed = !(PIND & (1<<PD2));
		// assign inverse to LED (pressed ? LED ON)
		if (pressed) PORTB |=  (1<<PB5);
		else         PORTB &= ~(1<<PB5);
	}
}