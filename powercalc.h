#ifndef POWERCALC_H
#define POWERCALC_H

#include <avr/io.h>
#include <stdint.h>

// Power calculation configuration
#define POWER_BUFFER_SIZE 100   // Buffer for power calculations

// Function declarations
uint16_t approximate_current_at_V(volatile uint16_t i_samples[], uint8_t i);
uint16_t approximate_voltage_at_I(volatile uint16_t v_samples[], uint8_t i);
void powercalc_init(void);
void powercalc_update_samples(uint16_t vmeas_adc, uint16_t imeas_adc, uint16_t offset_adc);

// New 24-sample calculation functions
void calculate_sample_metrics(void);
uint16_t get_average_power_24(void);
uint16_t get_rms_voltage_24(void);
uint16_t get_peak_current_24(void);

// Thread-safe display buffer functions
uint16_t get_display_power(void);
uint16_t get_display_voltage(void);
uint16_t get_display_current(void);
uint8_t is_display_data_ready(void);
void set_display_data_ready(uint8_t ready);
uint8_t get_adc_sample_complete(void);
void set_adc_sample_complete(uint8_t complete);
uint8_t get_ready_for_new_sample(void);
void set_ready_for_new_sample(uint8_t ready);


#endif // POWERCALC_H
