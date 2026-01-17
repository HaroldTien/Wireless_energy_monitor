/*
 * Part2.c - Main Application
 *
 * Main application file for Lab6_Display Part2
 * Demonstrates 4-digit 7-segment display control using modular structure
 * 
 * Created: 9/27/2025 11:51:31 PM
 * Author : harol
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"          // F_CPU must be defined before util/delay.h
#include <util/delay.h>      // Now F_CPU is available
#include "display.h"
#include "seven_seg.h"

int main(void)
{
    uint16_t counter = 0;
    
    // Initialize the display system
    init_display();
    
    // Initialize Timer0 for 10ms display scanning
    init_timer0_for_display();
    
    // Enable global interrupts
    sei();
    
    // Initialize display with counter value 0
    seperate_and_load_characters(counter);
    
    /* Main loop */
    while (1) {
        // Increment counter every 400ms
        _delay_ms(400);
        
        // Increment counter (0 to 9999)
        counter++;
        if (counter > 9999) {
            counter = 0;  // Reset to 0 when reaching 9999
        }
        
        // Update display with new counter value
        seperate_and_load_characters(counter);
        
        // The Timer0 ISR will automatically handle display scanning
        // No need to call display functions in main loop
    }
}

    