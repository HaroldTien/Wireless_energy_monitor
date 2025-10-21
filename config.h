#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>
#include <stdint.h>

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================

// Clock frequency
#define F_CPU 2000000UL   // 2 MHz

// ============================================================================
// ADC CONFIGURATION
// ============================================================================

// ADC Reference and Resolution
#define ADC_VREF 5000        // mV
#define ADC_RESOLUTION 10    // bits
#define ADC_MAX_VALUE 1023   // 2^10 - 1

// ADC Prescaler (125kHz ADC clock)
#define ADC_PRESCALER 16  // 2MHz / 16 = 125kHz

// ADC Channels
#define ADC_CH_VMEAS 0     // PC0 - Voltage measurement
#define ADC_CH_IMEAS 1     // PC1 - Current measurement  
#define ADC_CH_OFFSET 2    // PC2 - Offset reference

// ============================================================================
// UART CONFIGURATION
// ============================================================================

// UART Settings
#define UART_BAUD_RATE 9600
#define UART_BAUD_PRESCALER 12   // For 2MHz: 2000000/(16*9600) - 1 = 12.02 ≈ 12

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================

// Display Settings
#define DISPLAY_DIGITS 4
#define DISPLAY_SEGMENTS 8

// Shift Register Control Pins
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

// Digit Enable Pins
#define DIGIT_ENABLE_PORT PORTD
#define DIGIT_ENABLE_DDR  DDRD
#define DIGIT_ENABLE_PINS ((1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7))

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


// 7-segment display patterns (common cathode)
#define SEG_A (1<<0)
#define SEG_B (1<<1) 
#define SEG_C (1<<2)
#define SEG_D (1<<3)
#define SEG_E (1<<4)
#define SEG_F (1<<5)
#define SEG_G (1<<6)
#define SEG_DP (1<<7)

// ============================================================================
// ZERO-CROSSING DETECTION
// ============================================================================

// Zero-crossing pin
#define ZERO_CROSS_PIN PD2
#define ZERO_CROSS_PORT PORTD
#define ZERO_CROSS_DDR DDRD
#define ZERO_CROSS_PIN_REG PIND

// ============================================================================
// ENERGY MONITOR CONSTANTS
// ============================================================================

// Sampling and Processing
#define SAMPLE_BUFFER_SIZE 37

// Update Intervals
#define DISPLAY_UPDATE_MS 1000

// Hardware Scaling Factors
#define VOLTAGE_DIVIDER_RATIO 21
#define CURRENT_SHUNT_RESISTOR 0.545  // Ω
#define CURRENT_OPAM_GAIN 2.10

#endif // CONFIG_H