/*
 * Lab4_microchip.c
 *
 * Created: 9/15/2025 10:49:26 AM
 * Author : harol
 */ 

// main.c
#include <stdio.h>
#include <avr/io.h>
#include "common.h"
#include "adc.h"
#include "uart.h"
#include <util/delay.h>

#include <stdint.h>


#define ADC_VREF_MV             5000.0f // ADC reference voltage in millivolts
#define ADC_MAX_COUNT           1024.0f // Maximum ADC count (2^10)
#define DC_OFFSET_MV            2500.0f // DC offset from your signal conditioning in mV
#define VOLTAGE_SCALING_FACTOR  14.1f   // Your voltage sensor's scaling factor (V_actual / V_sensed)
#define CURRENT_SCALING_FACTOR  2.0f    // Your current sensor's scaling factor (I_actual / V_sensed)

float get_actual_mv(uint16_t raw_sample, float scaling_factor);
float calculate_real_power_corrected(uint16_t v_samples[], uint16_t i_samples[], uint8_t num_samples);


int main(void)
{
	adc_init();         // from your earlier part (Vref=AVCC, prescaler/16, etc.)
	uart_init(9600);	
	
	uint16_t ch0_mv[40];
	uint16_t ch1_mv[40];
	char buffer[32];
	
	
	for (uint8_t i = 0; i < 40; i++) {
		uint16_t raw_0 = adc_read(0);                 // ADC0
		ch0_mv[i]   = adc_convert_mv(raw_0);

		uint16_t raw_1 = adc_read(1);                 // ADC1
		ch1_mv[i]   = adc_convert_mv(raw_1);
	}

	

	uart_transmit_string("ADC0_mV,ADC1_mV\r\n");

    // Print all 40 rows using snprintf
    for (uint8_t i = 0; i < 40; i++) {
	    snprintf(buffer, sizeof(buffer), "%u,%u\r\n", ch0_mv[i], ch1_mv[i]);
	    uart_transmit_string(buffer);
    }
	while (1) { }                               // done

}


// Helper function to convert a raw ADC value to its real-world AC value
// This simplifies the main calculation function
float get_actual_mv(uint16_t raw_sample, float scaling_factor) {
	float sensed_voltage_mv = raw_sample * (ADC_VREF_MV / ADC_MAX_COUNT);
	float ac_component_mv = sensed_voltage_mv - DC_OFFSET_MV;
	return ac_component_mv * scaling_factor;
}


/**
 * @brief Calculates average real power, correcting for sample time delay using linear approximation.
 * @param v_samples         Array of raw ADC samples for voltage (ADC0).
 * @param i_samples         Array of raw ADC samples for current (ADC1).
 * @param num_samples       The number of samples in each array.
 * @return float            The calculated average real power in Watts.
 */
float calculate_real_power_corrected(uint16_t v_samples[], uint16_t i_samples[], uint8_t num_samples) {
    int64_t power_sum_microwatts = 0;

    for (uint8_t i = 0; i < num_samples; i++) {
        // --- 1. Convert the necessary raw ADC samples to real-world units (mV and mA) ---
        float actual_v_mv = get_actual_mv(v_samples[i], VOLTAGE_SCALING_FACTOR);
        float actual_i_ma = get_actual_mv(i_samples[i], CURRENT_SCALING_FACTOR);

        // --- 2. Estimate the "missing" samples using linear approximation ---
        
        // Estimate the current value at the *exact time the voltage was sampled*.
        // We do this by averaging the current sample before it and the current sample at that point.
        float estimated_i_ma;
        if (i == 0) {
            estimated_i_ma = actual_i_ma; // Edge case: For the first sample, no previous data exists.
        } else {
            float prev_actual_i_ma = get_actual_mv(i_samples[i-1], CURRENT_SCALING_FACTOR);
            estimated_i_ma = (prev_actual_i_ma + actual_i_ma) / 2.0f;
        }

        // --- 3. Calculate the instantaneous power product and add to the sum ---
        // P_inst = V_inst * I_inst. (mV * mA = microwatts)
        // We use the actual voltage sample with our *estimated* current sample.
        power_sum_microwatts += (int64_t)(actual_v_mv * estimated_i_ma);
    }

    // --- 4. Calculate the average power ---
    float average_power_microwatts = (float)power_sum_microwatts / num_samples;
    
    // Convert from microwatts to watts for the final result
    float average_power_watts = average_power_microwatts / 1000000.0f;

    return average_power_watts;
}