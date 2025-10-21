
#include "timer.h"
#include "adc.h"
#include "display.h"
#include <avr/interrupt.h>

// Global variables (none needed for current implementation)


/*
 * Initialize Timer0 for 10ms interrupt
 * Timer0 will trigger ISR every 10ms to update display
 */
void timer0_init(void)
{
    // Configure Timer0 for CTC mode (Clear Timer on Compare)
    TCCR0A = (1 << WGM01);  // CTC mode
    
    // Set prescaler to 1024 and start timer
    // For 2MHz clock: 2MHz / 1024 = 1953.125 Hz
    // To get 10ms: 1953.125 / 100 = 19.53 ≈ 20
    // OCR0A = 19 for 10ms interrupt
    TCCR0B = (1 << CS02) | (1 << CS00);  // Prescaler 1024
    OCR0A = 19;  // Compare value for ~10ms interrupt
    
    // Enable Timer0 compare A interrupt
    TIMSK0 = (1 << OCIE0A);
}

void timer1_init(void){
	TCCR1A = 0;
	TCCR1B = 0;
	
	// set timer1 to CTC mode
	TCCR1B |= (1 << WGM12);
	
	//set the compare value for the 108us interval
	OCR1A = 215; 
	//set the compare match B value which is for ADC auto-trigger.
	OCR1B = 215;
	
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));	
}

void timer1_start(void){
	//reset timer counter to 0
	TCNT1 = 0;
	
	//set prescaler to 1
	TCCR1B |= (1 << CS10);
	TCCR1B &= ~((1 << CS12) | (1 << CS11));
}

void timer1_stop(void){
	//stop timer1
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
}

void timer1_clear_compare_match_b_flag(void)
{
    // Clear the Timer1 Compare Match B interrupt flag by writing a logic one to OCF1B
    TIFR1 |= (1 << OCF1B);
}

// Timer0 Compare A Interrupt Service Routine
ISR(TIMER0_COMPA_vect)
{
    send_next_character_to_display();
}
