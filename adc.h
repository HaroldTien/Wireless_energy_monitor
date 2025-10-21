#ifndef ADC_H
#define ADC_H

#include <avr/io.h>
#include <stdint.h>
#include "config.h"

// ADC Configuration
// ADC_PRESCALER is defined in config.h
#define ADC_REFERENCE 0x01 // AVCC reference (5V)
#define ADC_CHANNELS 3     // Vmeas, Imeas, Offset

// ADC Channel definitions
#define ADC_CH_VMEAS 0     // PC0 - Voltage measurement
#define ADC_CH_IMEAS 1     // PC1 - Current measurement  
#define ADC_CH_OFFSET 2    // PC2 - Offset reference



// Function declarations
void adc_init(void);
void adc_start_conversion(uint8_t channel);
void adc_stop_conversion(void);
uint16_t adc_get_result(void);
uint8_t adc_is_conversion_complete(void);
uint8_t adc_is_conversion_running(void);
void adc_switch_channel(uint8_t channel);
void adc_enable_auto_trigger(void);
void adc_disable_auto_trigger(void);
// Variables for 24-sample collection
extern volatile uint16_t voltage_samples_raw[SAMPLE_BUFFER_SIZE];
extern volatile uint16_t current_samples_raw[SAMPLE_BUFFER_SIZE];
extern volatile uint16_t offset_sample; 
extern volatile uint8_t sample_count;
extern volatile uint8_t adc_sample_complete;
extern volatile uint8_t current_adc_channel;

#endif // ADC_H