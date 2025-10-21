#include "powercalc.h"
#include "config.h"
#include "adc.h"
#include "uart.h"
#include <avr/interrupt.h>
#include <math.h>

// Global variables for power calculations

// New variables for 24-sample calculations
volatile uint16_t average_power_raw = 0.0f;
volatile uint16_t rms_voltage_raw = 0.0f;
volatile uint16_t peak_current_raw = 0.0f;

// Display buffer for thread-safe display updates
volatile uint16_t display_power = 0.0f;
volatile uint16_t display_voltage = 0.0f;
volatile uint16_t display_current = 0.0f;

// Buffers for power calculations
static uint16_t power_buffer[POWER_BUFFER_SIZE];
static uint8_t power_buffer_index = 0;
static uint8_t power_buffer_full = 0;

volatile uint8_t display_data_ready = 0;
volatile uint8_t ready_for_new_sample = 1;

// Power calculation initialization
void powercalc_init(void)
{
    // Initialize power buffer
    for (uint8_t i = 0; i < POWER_BUFFER_SIZE; i++) {
        power_buffer[i] = 0.0f;
    }
    
    // Initialize variables
    power_buffer_index = 0;
    power_buffer_full = 0;
    
    // Initialize display buffer (ensure zeros are displayed initially)
    display_power = 0.0f;
    display_voltage = 0.0f;
    display_current = 0.0f;
	display_data_ready = 0;
	ready_for_new_sample = 1;
}

/**
 * @brief Approximates the current sample (I_L_bar) at the time of a voltage sample.
 * This function assumes 'i' is always > 0.
 */
uint16_t approximate_current_at_V(volatile uint16_t i_samples[], uint8_t i)
{
    // Implements the formula: I_L_bar[i] = (I_L[i-1] + I_L[i]) / 2
    return (i_samples[i - 1] + i_samples[i]) / 2;
}

/**
 * @brief Approximates the voltage sample (V_AC_bar) at the time of a current sample.
 * This function assumes 'i' is always < (size - 1).
 */
uint16_t approximate_voltage_at_I(volatile uint16_t v_samples[], uint8_t i)
{
    // Implements the formula: V_AC_bar[i] = (V_AC[i] + V_AC[i+1]) / 2
    return (v_samples[i] + v_samples[i + 1]) / 2;
}



// Calculate power metrics from 24 samples
void calculate_sample_metrics(void)
{
	// --- MAIN CALCULATION STEP ---
	uint16_t power_sum_raw = 0;
	uint32_t sum_voltage_squared_raw = 0;
	uint16_t max_current_signed_raw = 0;

	// DEBUG: Print offset_sample value
	usart_transmit_string("Offset: ");
	usart_transmit_float(offset_sample, 0);
	usart_transmit_string("\r\n");

	for( uint8_t i = 0; i < (uint8_t)SAMPLE_BUFFER_SIZE; i++ ){
		// DEBUG: Print raw values for first few samples
		if (i < 3) {
			usart_transmit_string("Sample[");
			usart_transmit_float(i, 0);
			usart_transmit_string("]: V_raw=");
			usart_transmit_float(voltage_samples_raw[i], 0);
			usart_transmit_string(" I_raw=");
			usart_transmit_float(current_samples_raw[i], 0);
			usart_transmit_string("\r\n");
		}
		
		uint16_t v_sample = voltage_samples_raw[i] - offset_sample;
		uint16_t i_sample = current_samples_raw[i] - offset_sample;
		
		// DEBUG: Print calculated values for first few samples
		if (i < 3) {
			usart_transmit_string("  After subtract: v_sample=");
			usart_transmit_float(v_sample, 0);
			usart_transmit_string(" i_sample=");
			usart_transmit_float(i_sample, 0);
			usart_transmit_string("\r\n");
		}
		// 1. Average Power using Linear Approximation (only on inner samples)
		if (i > 0 && i < (uint8_t)SAMPLE_BUFFER_SIZE - 1) {
			// Call helper functions to get the approximated values
			uint16_t v_bar = approximate_voltage_at_I(voltage_samples_raw, i);
			uint16_t i_bar = approximate_current_at_V(current_samples_raw, i);
					
			// Sum the two power estimates
			power_sum_raw += (v_sample * i_bar) + (v_bar * i_sample);

		}

		// 2. RMS Voltage (using all samples)
		sum_voltage_squared_raw += v_sample * v_sample;
				
		// 3. Peak Current (using all  samples)
		if (i_sample > max_current_signed_raw) {
			max_current_signed_raw = i_sample;
		}
	}
	usart_transmit_string(" ----->");
	usart_transmit_float(max_current_signed_raw ,0);
	usart_transmit_string(" \r\n");

	
	uint8_t power_sample_count = SAMPLE_BUFFER_SIZE - 2;
	average_power_raw = power_sum_raw / (2.0 * (uint16_t)power_sample_count);
	rms_voltage_raw = sqrt(sum_voltage_squared_raw / (uint16_t)SAMPLE_BUFFER_SIZE);
	peak_current_raw = max_current_signed_raw;
	


	//Covert ADC value to actual values and Atomic copy to display buffer
	uint16_t average_power_sample_W = average_power_raw * (uint16_t)(ADC_VREF / ADC_MAX_VALUE)/1000; // in W
	uint16_t rms_voltage_sample_mV = rms_voltage_raw * (uint16_t)(ADC_VREF / ADC_MAX_VALUE)/1000; // in V
	uint16_t peak_current_sample_mA = peak_current_raw *(uint16_t)(ADC_VREF / ADC_MAX_VALUE)/1000; // in mA
	
	
	cli();
	display_power = average_power_sample_W * (uint16_t)VOLTAGE_DIVIDER_RATIO * (uint16_t)CURRENT_OPAM_GAIN * (uint16_t)(CURRENT_SHUNT_RESISTOR*1000)/1000;
	display_voltage = rms_voltage_sample_mV* (uint16_t)VOLTAGE_DIVIDER_RATIO;
	display_current = peak_current_sample_mA / ((uint16_t)CURRENT_OPAM_GAIN * (uint16_t)(CURRENT_SHUNT_RESISTOR*1000)/1000);
	set_display_data_ready(1);
	set_ready_for_new_sample(1);
	sei();
}


// Thread-safe display buffer getters
uint16_t get_display_power(void)
{
    return display_power;
}

uint16_t get_display_voltage(void)
{
    return display_voltage;
}

uint16_t get_display_current(void)
{
    return display_current;
}

void set_display_data_ready(uint8_t ready)
{
    display_data_ready = ready;
}

uint8_t is_display_data_ready(void)
{
    return display_data_ready;
}
uint8_t get_adc_sample_complete(void)
{
    return adc_sample_complete;
}
void set_adc_sample_complete(uint8_t complete)
{
    adc_sample_complete = complete;
}
uint8_t get_ready_for_new_sample(void)
{
    return ready_for_new_sample;
}
void set_ready_for_new_sample(uint8_t ready)
{
    ready_for_new_sample = ready;
}