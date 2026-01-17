/*
 * display.h
 * 
 * Header file for 7-segment display control functions
 * Contains function declarations and constants for display operations
 * 
 * Created: 9/27/2025
 * Author: harol
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>
#include <stdint.h>

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

/*
 * Initialize display control pins
 * Configures shift register control pins and digit selection pins as outputs
 */
void init_display(void);

/*
 * Send character data to display via shift register (Modified for 4-digit scanning)
 * Controls the 74HC595 shift register to set segment patterns
 * This function now scans through the 4 digits automatically
 * Called from Timer0 ISR every 10ms
 */
void send_next_character_to_display(void);

/*
 * Populate the array 'disp_characters[]' by separating the four digits of 'number' 
 * and then looking up the segment pattern from 'seg_pattern[]'
 * @param number: 16-bit number to display (0-9999)
 */
void seperate_and_load_characters(uint16_t number);

/*
 * Initialize Timer0 for 10ms interrupt
 * Timer0 will trigger ISR every 10ms to update display
 */
void init_timer0_for_display(void);

#endif // DISPLAY_H
