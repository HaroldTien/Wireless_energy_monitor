#include "config.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>


#include "uart.h"
#include "adc.h"
#include "timer.h"
#include "display.h"
#include "powercalc.h"
#include "int0.h"




// Global variables for timing (none needed with simple delay approach)

int main(void)
{
    // Initialize hardware peripherals
    usart_init(UART_BAUD_PRESCALER);
    adc_init();
    init_display();  // Initialize display FIRST to configure PORTD pins properly
    timer0_init();  // Timer0 handles display multiplexing
    timer1_init();  // Timer1 handles ADC sampling
    int0_init();    // INT0 triggers new ADC sequences (must be after init_display)
    init_scrolling_display();
    powercalc_init();

    // Enable global interrupts
    sei();

    // Main application loop
    while (1)
    {		
      
      // Check if 24 samples are complete and ADC is idle
      if ( get_adc_sample_complete() == 1) {
        usart_transmit_string("ADC sample complete!\r\n");
        calculate_sample_metrics();
        set_adc_sample_complete(0);
      }
          
      // Update scrolling display every 1 second using simple delay
      // Display keeps showing last calculated values during new sampling
      update_scrolling_display();
      usart_send_power_data(); // Send data via UART every 1 second
      _delay_ms(DISPLAY_UPDATE_MS); // Wait exactly 1 second
    }       
}
