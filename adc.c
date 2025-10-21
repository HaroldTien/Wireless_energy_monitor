#include "adc.h"
#include "powercalc.h"
#include "config.h"
#include <avr/interrupt.h>
#include "timer.h"

// Global variables
volatile uint8_t adc_conversion_complete = 0;
volatile uint8_t current_adc_channel = 0;

// Variables for 24-sample collection
volatile uint16_t voltage_samples_raw[SAMPLE_BUFFER_SIZE] = {0};
volatile uint16_t current_samples_raw[SAMPLE_BUFFER_SIZE] = {0};
volatile uint16_t offset_sample = 0;
volatile uint8_t sample_count = 0;
volatile uint8_t adc_sample_complete = 0;



// ADC Initialization
void adc_init(void)
{
    // Set AVCC as reference, right-adjust result
    ADMUX = (ADC_REFERENCE << REFS0); 
    
    // Enable ADC, set prescaler to 16 (125kHz ADC clock)
    ADCSRA = (1 << ADEN) | (1 << ADPS2);
	
	// Enable ADC interrupts and auto-trigger
	ADCSRA |= (1 << ADIE);
	
	ADCSRB |= (1 << ADTS0) | (1 << ADTS2);
	ADCSRB &= ~(1 << ADTS1);
    
    // Initialise voltage_samples, current_samples and offset_sample
    for (uint8_t i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        voltage_samples_raw[i] = 0;
        current_samples_raw[i] = 0;
    }
    offset_sample = 0;
    adc_sample_complete = 0;
    current_adc_channel = 0;
}

void adc_enable_auto_trigger(void)
{
    ADCSRA |= (1 << ADATE);
}

void adc_disable_auto_trigger(void)
{
    ADCSRA &= ~(1 << ADATE);
}



// Function to switch ADC channel
void adc_switch_channel(uint8_t channel)
{
    ADMUX = (ADC_REFERENCE << REFS0) | (channel & 0x0F);
}

// Start an ADC conversion on the selected channel (non-blocking)
// This will select the ADC reference and channel, then initiate a conversion.
// The conversion result can be read later, or handled via interrupt if enabled.
void adc_start_conversion(uint8_t channel)
{
    // Select ADC reference and channel
    adc_switch_channel(channel);
    ADCSRA |= (1 << ADSC);
}

// stop ADC free running mode
void adc_stop_conversion(void){
	ADCSRA &= ~(1 <<ADATE);
}

// Check if ADC conversion is currently running
// Returns 1 if conversion is running, 0 if idle
uint8_t adc_is_conversion_running(void)
{
    return (ADCSRA & (1 << ADSC)) ? 1 : 0;
}


// ADC Complete Interrupt Service Routine
ISR(ADC_vect)
{
    // Store result based on current channel being sampled
    if (current_adc_channel == 0) {
        // Voltage sample
        voltage_samples_raw[sample_count] = ADC;
        current_adc_channel = 1; // Next sample will be current
    } else if (current_adc_channel == 1) {
        // Current sample
        current_samples_raw[sample_count] = ADC;
		current_adc_channel = 0; // Next sample will be voltage
		sample_count++;      // Increment after all three channels are sampled
		if(sample_count >= SAMPLE_BUFFER_SIZE){
			current_adc_channel = 2;
		}
    } else if (current_adc_channel == 2) {
        // Offset sample
        offset_sample = ADC;
		set_adc_sample_complete(1);
        timer1_stop();  // All samples collected, stop Timer1
        adc_disable_auto_trigger();
		return;
    }
    
    // Start next conversion on the next channel using existing function
    if (current_adc_channel == 0) {
        adc_switch_channel(ADC_CH_VMEAS);
    } else if (current_adc_channel == 1) {
        adc_switch_channel(ADC_CH_IMEAS);
    } else if(current_adc_channel == 2 ){
		adc_switch_channel(ADC_CH_OFFSET);
	}
}
