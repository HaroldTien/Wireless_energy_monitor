#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>
#include <stdint.h>

// Display Configuration
#define DISPLAY_DIGITS 4
#define DISPLAY_SEGMENTS 8

// Pin definitions for shift register control
#define SHIFT_CLOCK_PORT PORTC
#define SHIFT_CLOCK_PIN  PC3
#define SHIFT_DATA_PORT  PORTC  
#define SHIFT_DATA_PIN   PC4
#define SHIFT_LATCH_PORT PORTC
#define SHIFT_LATCH_PIN  PC5

// Pin definitions for digit enable
#define DIGIT_ENABLE_PORT PORTD
#define DIGIT_ENABLE_DDR  DDRD
// DIGIT_ENABLE_PINS is defined in config.h

// 7-segment display patterns (common cathode)
#define SEG_A (1<<0)
#define SEG_B (1<<1) 
#define SEG_C (1<<2)
#define SEG_D (1<<3)
#define SEG_E (1<<4)
#define SEG_F (1<<5)
#define SEG_G (1<<6)
#define SEG_DP (1<<7)

// Function declarations
void init_display(void);
void display_update(void);
void display_show_number(uint16_t number);
void display_show_float(float value, uint8_t decimals);
void display_clear(void);
void display_set_digit(uint8_t digit, uint8_t value);
void seperate_and_load_characters(uint16_t number, uint8_t decimal_pos);
void send_next_character_to_display(void);

// Scrolling display functions
void init_scrolling_display(void);
void update_scrolling_display(void);
uint8_t get_scroll_mode(void);
void display_no_signal(void);

// Global variables
extern volatile uint8_t display_buffer[DISPLAY_DIGITS];
extern volatile uint8_t current_digit;



#endif // DISPLAY_H