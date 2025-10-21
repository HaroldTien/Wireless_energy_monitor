#include "int0.h"
#include "adc.h"
#include "config.h"
#include <avr/interrupt.h>
#include "powercalc.h"
#include "uart.h"
#include "timer.h"

// INT0 Initialization
void int0_init(void)
{
    // Configure PD2 (INT0) as input
    ZERO_CROSS_DDR &= ~(1 << ZERO_CROSS_PIN);
    
    // Enable internal pull-up resistor
    ZERO_CROSS_PORT |= (1 << ZERO_CROSS_PIN);
    
    // Configure INT0 to trigger on rising edge
    EICRA |= (1 << ISC01) | (1 << ISC00);   // ISC01 = 1, ISC00 = 1 for rising edge
    
    // Enable INT0 interrupt
    EIMSK |= (1 << INT0);
}

// Global variables (none currently used)

// INT0 Interrupt Service Routine - Start new ADC conversion sequence
ISR(INT0_vect)
{
    // Only start new sequence if previous one is complete AND no ADC conversion is running
    if (get_ready_for_new_sample() == 1) {
        usart_transmit_string("INT0 triggered!\r\n");
        // Reset sampling for new sequence
        sample_count = 0;
        current_adc_channel = 0;
        // Reset the completion flag
        set_ready_for_new_sample(0);
        set_adc_sample_complete(0);

        timer1_start();  // Start Timer1 to trigger ADC conversions every 108us
        adc_enable_auto_trigger();
        timer1_clear_compare_match_b_flag();
        adc_start_conversion(ADC_CH_VMEAS); // Start conversion on voltage channel
    }
}
