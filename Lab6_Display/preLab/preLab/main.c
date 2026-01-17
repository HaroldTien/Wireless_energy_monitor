/*
 * preLab.c
 *
 * Created: 24/09/2025 2:20:35 PM
 * Author : thar970
 */ 
// ---------- Lab 6 Pre-Lab: single 7-seg, count 0..9, reset on button ----------
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>


#define DIGIT_DDR    DDRB
#define DIGIT_PORT   PORTB
#define DIGIT_DS1    PB0
#define DIGIT_DS2    PB1

/* Segment lines (common-cathode, active HIGH)
 *   a..f -> PC0..PC5
 *   g    -> PB2
 */
#define SEG_DDRC        DDRC
#define SEG_PORTC       PORTC
#define SEG_PORTC_MASK  0x3F    // PC0..PC5

#define SEG_DDRB        DDRB
#define SEG_PORTB       PORTB
#define SEG_G_BIT       PB4     // segment g on PB4

/* Button on PB7 with external pull-up (pressed = LOW) */
#define BTN_PINREG   PINB
#define BTN_DDR      DDRB
#define BTN_BIT      PB7


volatile uint8_t counter = 0;


static const uint8_t seg_pattern[10] = {
	/*0*/ 0b00111111, // a b c d e f
	/*1*/ 0b00000110, //   b c
	/*2*/ 0b01011011, // a b   d e   g
	/*3*/ 0b01001111, // a b c d     g
	/*4*/ 0b01100110, //   b c     f g
	/*5*/ 0b01101101, // a   c d   f g
	/*6*/ 0b01111101, // a   c d e f g
	/*7*/ 0b00000111, // a b c
	/*8*/ 0b01111111, // a b c d e f g
	/*9*/ 0b01101111  // a b c d   f g
};

void timer0_init(void) {
	// Set Timer0 to CTC (Clear Timer on Compare Match) mode
	TCCR0A |= (1 << WGM01);

	// Set prescaler to 1024
	TCCR0B |= (1 << CS02) | (1 << CS00);

	// Set the compare value for a 10ms interrupt with F_CPU = 16MHz
	// OCR0A = (Target Time * F_CPU / Prescaler) - 1
	// OCR0A = (0.01s * 16,000,000Hz / 1024) - 1 = 155.25 -> 155
	OCR0A = 155;

	// Enable the Timer0 Compare Match A interrupt
	TIMSK0 |= (1 << OCIE0A);
}

void display_digit(uint8_t pattern) {
	// Set segments a-f (bits 0-5) on PORTC
	SEG_PORTC = (pattern & 0x3F);

	// Set segment g (bit 6) on PORTB
	if (pattern & (1 << 6)) {
		SEG_PORTB |= (1 << SEG_G_BIT);
		} else {
		SEG_PORTB &= ~(1 << SEG_G_BIT);
	}
}

// --- Initialize GPIO pins ---
void io_init(void) {
	// Configure segment pins as outputs
	SEG_DDRC |= 0x3F;           // PC0-PC5
	SEG_DDRB |= (1 << SEG_G_BIT); // PB4
	
	// Configure digit select pins as outputs
	DIGIT_DDR |= (1 << DIGIT_DS1) | (1 << DIGIT_DS2);
	
	// Initialize with both digits OFF
	DIGIT_PORT |= (1 << DIGIT_DS1) | (1 << DIGIT_DS2);
}



bool button_is_pressed(void) {
	if (!(BTN_PINREG & (1 << BTN_BIT))) {
		_delay_ms(20); // Debounce delay
		if (!(BTN_PINREG & (1 << BTN_BIT))) {
			return true;
		}
	}
	return false;
}

// --- Interrupt Service Routine for Timer0 Compare Match A ---
// This function runs automatically every 10ms
ISR(TIMER0_COMPA_vect) {
	static uint8_t current_digit = 1; // Flag to track which digit to update

	// Separate the tens and ones digits from the global counter
	uint8_t tens_digit = counter / 10;
	uint8_t ones_digit = counter % 10;

	// Disable both digits first to prevent ghosting
	DIGIT_PORT |= (1 << DIGIT_DS1) | (1 << DIGIT_DS2);

	if (current_digit == 1) {
		// Display the tens digit on Ds1
		display_digit(seg_pattern[tens_digit]);
		DIGIT_PORT &= ~(1 << DIGIT_DS1); // Enable Ds1
		current_digit = 2; // Set to update the other digit next time
		} else {
		// Display the ones digit on Ds2
		display_digit(seg_pattern[ones_digit]);
		DIGIT_PORT &= ~(1 << DIGIT_DS2); // Enable Ds2
		current_digit = 1; // Set to update the other digit next time
	}
}


int main(void) {
    /* Init peripherals */
	io_init();
	
    timer0_init();

    /* Button input (external pull-up already on schematic) */
    BTN_DDR &= ~(1<<BTN_BIT);   // input

    sei();  // enable global interrupts (starts multiplexing)

	while (1) {
		_delay_ms(1000);
		if (button_is_pressed()) {
			counter = 0; // Use the global counter
		} else {
			counter++; // Use the global counter
			if (counter > 99) {
				counter = 0;
			}
		}
	}
}
