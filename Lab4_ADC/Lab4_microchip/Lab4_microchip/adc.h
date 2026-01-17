/*
 * adc.h
 *
 * Created: 9/15/2025 10:59:13 AM
 *  Author: harol
 */ 


// adc.h
#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>

void     adc_init(void);
uint16_t adc_read(uint8_t chan);
uint16_t adc_convert_mv(uint16_t value);


#endif
