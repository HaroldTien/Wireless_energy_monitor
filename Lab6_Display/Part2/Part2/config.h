/*
 * Pin Definitions for Lab6_Display Part2
 * Based on ATMEGA328P circuit diagrams
 * 
 * Created: 9/27/2025
 * Author: harol
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>


// System clock frequency: 2MHz (as required for labs and project)
#ifndef F_CPU
#define F_CPU 2000000UL
#endif

// =============================================================================
// 7-SEGMENT DISPLAY CONTROL PINS (via 74HC595 Shift Register)
// =============================================================================

// 74HC595 Shift Register Control Pins
#define SHIFT_CLOCK_PORT    PORTC
#define SHIFT_CLOCK_DDR     DDRC
#define SHIFT_CLOCK_PIN     PC3      // Pin 26 - SH_CP (Shift Register Clock)
#define SHIFT_CLOCK_BIT     PC3

#define SHIFT_DATA_PORT     PORTC
#define SHIFT_DATA_DDR      DDRC
#define SHIFT_DATA_PIN      PC4      // Pin 27 - SH_DS (Serial Data Input)
#define SHIFT_DATA_BIT      PC4

#define SHIFT_LATCH_PORT    PORTC
#define SHIFT_LATCH_DDR     DDRC
#define SHIFT_LATCH_PIN     PC5      // Pin 28 - SH_ST (Storage Register Clock)
#define SHIFT_LATCH_BIT     PC5

// 7-Segment Display Segment Mapping (via 74HC595 outputs)
// Q0-Q7 outputs from 74HC595 connected to display segments through 330R resistors
#define SEG_A_Q0    0       // Q0 -> Segment A (top horizontal)
#define SEG_B_Q1    1       // Q1 -> Segment B (top-right vertical)
#define SEG_C_Q2    2       // Q2 -> Segment C (bottom-right vertical)
#define SEG_D_Q3    3       // Q3 -> Segment D (bottom horizontal)
#define SEG_E_Q4    4       // Q4 -> Segment E (bottom-left vertical)
#define SEG_F_Q5    5       // Q5 -> Segment F (top-left vertical)
#define SEG_G_Q6    6       // Q6 -> Segment G (middle horizontal)
#define SEG_DP_Q7   7       // Q7 -> Decimal Point

// =============================================================================
// 7-SEGMENT DISPLAY DIGIT SELECT PINS
// =============================================================================

// Digit Selection Pins (Individual control for each digit)
#define DIGIT1_PORT     PORTD
#define DIGIT1_DDR      DDRD
#define DIGIT1_PIN      PD4      // Pin 2 - Ds1 (First digit)
#define DIGIT1_BIT      PD4

#define DIGIT2_PORT     PORTD
#define DIGIT2_DDR      DDRD
#define DIGIT2_PIN      PD5      // Pin 9 - Ds2 (Second digit)
#define DIGIT2_BIT      PD5

#define DIGIT3_PORT     PORTD
#define DIGIT3_DDR      DDRD
#define DIGIT3_PIN      PD6      // Pin 10 - Ds3 (Third digit)
#define DIGIT3_BIT      PD6

#define DIGIT4_PORT     PORTD
#define DIGIT4_DDR      DDRD
#define DIGIT4_PIN      PD7      // Pin 11 - Ds4 (Fourth digit)
#define DIGIT4_BIT      PD7



#endif // CONFIG_H
