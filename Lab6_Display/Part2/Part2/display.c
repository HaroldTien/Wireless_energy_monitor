/*
 * display.c
 * 
 * Source file for 7-segment display control functions
 * Implements all display-related functionality
 * 
 * Created: 9/27/2025
 * Author: harol
 */

#include "display.h"
#include "config.h"
#include "seven_seg.h"
#include <avr/interrupt.h>

// =============================================================================
// GLOBAL VARIABLES FOR 4-DIGIT COUNTER
// =============================================================================

// Array containing which segments to turn on to display a number between 0 to 9
// As an example seg_pattern[0] is populated with pattern to display number '0'
// TODO: Populate this array using your answer to QP.1
const uint8_t seg_pattern[10] = {
    0x3F,  // 0 - segments A,B,C,D,E,F
    0x06,  // 1 - segments B,C
    0x5B,  // 2 - segments A,B,G,E,D
    0x4F,  // 3 - segments A,B,G,C,D
    0x66,  // 4 - segments F,G,B,C
    0x6D,  // 5 - segments A,F,G,C,D
    0x7D,  // 6 - segments A,F,G,E,D,C
    0x07,  // 7 - segments A,B,C
    0x7F,  // 8 - segments A,B,C,D,E,F,G
    0x6F   // 9 - segments A,B,C,D,F,G
};

// 4 characters to be displayed on Ds1 to Ds4
static volatile uint8_t disp_characters[4] = {0, 0, 0, 0};

// The current digit (e.g. the 1's, the 10's) of the 4-digit number we're displaying
static volatile uint8_t disp_position = 0;

// =============================================================================
// DISPLAY INITIALIZATION
// =============================================================================

/*
 * Initialize display control pins
 * Configures shift register control pins and digit selection pins as outputs
 */
void init_display(void)
{
    // Configure shift register control pins as outputs
    // SH_ST, SH_DS, SH_CP pins
    SHIFT_CLOCK_DDR |= (1 << SHIFT_CLOCK_BIT);   // SH_CP (PC3) as output
    SHIFT_DATA_DDR |= (1 << SHIFT_DATA_BIT);     // SH_DS (PC4) as output  
    SHIFT_LATCH_DDR |= (1 << SHIFT_LATCH_BIT);   // SH_ST (PC5) as output
    
    // Initialize shift register pins to LOW
    SHIFT_CLOCK_PORT &= ~(1 << SHIFT_CLOCK_BIT);
    SHIFT_DATA_PORT &= ~(1 << SHIFT_DATA_BIT);
    SHIFT_LATCH_PORT &= ~(1 << SHIFT_LATCH_BIT);
    
    // Configure digit selection pins as outputs
    // Ds1, Ds2, Ds3, Ds4 pins
    DIGIT1_DDR |= (1 << DIGIT1_BIT);     // Ds1 (PD4) as output
    DIGIT2_DDR |= (1 << DIGIT2_BIT);     // Ds2 (PD5) as output
    DIGIT3_DDR |= (1 << DIGIT3_BIT);     // Ds3 (PD6) as output
    DIGIT4_DDR |= (1 << DIGIT4_BIT);     // Ds4 (PD7) as output
    
    // Initialize all digit selection pins to HIGH (disabled)
    DIGIT1_PORT |= (1 << DIGIT1_BIT);    // Ds1 = HIGH (disabled)
    DIGIT2_PORT |= (1 << DIGIT2_BIT);    // Ds2 = HIGH (disabled)
    DIGIT3_PORT |= (1 << DIGIT3_BIT);    // Ds3 = HIGH (disabled)
    DIGIT4_PORT |= (1 << DIGIT4_BIT);    // Ds4 = HIGH (disabled)
}

// =============================================================================
// SHIFT REGISTER CONTROL
// =============================================================================

/*
 * Send character data to display via shift register (Modified for 4-digit scanning)
 * Controls the 74HC595 shift register to set segment patterns
 * This function now scans through the 4 digits automatically
 * Called from Timer0 ISR every 10ms
 */
void send_next_character_to_display(void)
{
    uint8_t segments;
    
    // Based on 'disp_position', load the digit to send to a local variable
    segments = disp_characters[disp_position];
    
    // Send this bit pattern to the shift-register as in Q2.2
    // Send 8 bits of segment data to shift register
    for (int i = 7; i >= 0; i--) {
        // Set data pin based on bit value
        if (segments & (1 << i)) {
            SHIFT_DATA_PORT |= (1 << SHIFT_DATA_BIT);   // Set data HIGH
        } else {
            SHIFT_DATA_PORT &= ~(1 << SHIFT_DATA_BIT);  // Set data LOW
        }
        
        // Pulse clock to shift data in
        SHIFT_CLOCK_PORT |= (1 << SHIFT_CLOCK_BIT);     // Clock HIGH
        SHIFT_CLOCK_PORT &= ~(1 << SHIFT_CLOCK_BIT);    // Clock LOW
    }
    
    // Disable all digits first
    DIGIT1_PORT |= (1 << DIGIT1_BIT);    // Ds1 = HIGH (disabled)
    DIGIT2_PORT |= (1 << DIGIT2_BIT);    // Ds2 = HIGH (disabled)
    DIGIT3_PORT |= (1 << DIGIT3_BIT);    // Ds3 = HIGH (disabled)
    DIGIT4_PORT |= (1 << DIGIT4_BIT);    // Ds4 = HIGH (disabled)
    
    // Latch the output by toggling SH_ST pin as in Q2.2
    SHIFT_LATCH_PORT |= (1 << SHIFT_LATCH_BIT);        // Latch HIGH
    SHIFT_LATCH_PORT &= ~(1 << SHIFT_LATCH_BIT);       // Latch LOW
    
    // Now, depending on the value of disp_position, enable the correct digit
    // Correct mapping: Ds4=units, Ds3=tens, Ds2=hundreds, Ds1=thousands
    switch (disp_position) {
        case 0:  // Thousands digit -> Ds1
            DIGIT1_PORT &= ~(1 << DIGIT1_BIT);  // Ds1 = LOW (active)
            break;
        case 1:  // Hundreds digit -> Ds2
            DIGIT2_PORT &= ~(1 << DIGIT2_BIT);  // Ds2 = LOW (active)
            break;
        case 2:  // Tens digit -> Ds3
            DIGIT3_PORT &= ~(1 << DIGIT3_BIT);  // Ds3 = LOW (active)
            break;
        case 3:  // Units digit -> Ds4
            DIGIT4_PORT &= ~(1 << DIGIT4_BIT);  // Ds4 = LOW (active)
            break;
    }
    
    // Increment 'disp_position' so the next of the 4 digits will be displayed 
    // when function is called again from ISR (reset 'disp_position' after 3)
    disp_position++;
    if (disp_position > 3) {
        disp_position = 0;  // Reset to 0 after displaying position 3
    }
}

// =============================================================================
// DIGIT SELECTION
// =============================================================================

/*
 * Populate the array 'disp_characters[]' by separating the four digits of 'number' 
 * and then looking up the segment pattern from 'seg_pattern[]'
 * @param number: 16-bit number to display (0-9999)
 */
void seperate_and_load_characters(uint16_t number)
{
    uint8_t digit;
    
    // Separate each digit from 'number' (e.g., 1230 yields '1', '2', '3', '0')
    // Correct mapping: Ds4=units, Ds3=tens, Ds2=hundreds, Ds1=thousands
    
    // Thousands digit -> Ds1 (position 0)
    digit = (number / 1000) % 10;
    disp_characters[0] = seg_pattern[digit];
    
    // Hundreds digit -> Ds2 (position 1)
    digit = (number / 100) % 10;
    disp_characters[1] = seg_pattern[digit];
    
    // Tens digit -> Ds3 (position 2)
    digit = (number / 10) % 10;
    disp_characters[2] = seg_pattern[digit];
    
    // Units digit -> Ds4 (position 3)
    digit = number % 10;
    disp_characters[3] = seg_pattern[digit];
}

// =============================================================================
// TIMER0 CONFIGURATION AND ISR
// =============================================================================

/*
 * Initialize Timer0 for 10ms interrupt
 * Timer0 will trigger ISR every 10ms to update display
 */
void init_timer0_for_display(void)
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

/*
 * Timer0 Compare A Interrupt Service Routine
 * Called every 10ms to update display
 */
ISR(TIMER0_COMPA_vect)
{
    // Call the display function to scan through digits
    send_next_character_to_display();
}
