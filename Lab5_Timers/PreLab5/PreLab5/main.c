/*
 * preLab5.c
 *
 * Created: 19/09/2025 4:17:43 PM
 * Author : thar970
 */ 
#define F_CPU 2000000UL 
#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
	DDRB = 0xFF;  // set to output
	DDRC = 0x00;  // set to input 
	DDRD = 0x00; //set to input
   
	PORTB = 0xFF; // turn off the LED initailly.
   
    while (1) 
    {
		PORTB = (1 << PORTB5);
		_delay_ms(375);
		PORTB = 0x00;
		_delay_ms(125);
				
    }
}

