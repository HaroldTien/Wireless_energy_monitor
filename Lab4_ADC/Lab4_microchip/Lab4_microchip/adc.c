/*
 * adc.c
 *
 * Created: 9/15/2025 10:58:12 AM
 *  Author: harol
 */ 
// adc.c


// adc.c
#include "common.h"
#include "adc.h"
#include <avr/io.h>

void adc_init(void)
{
    /* ADMUX – reference & channel selection
       REFS1:0 = 01 ? AVCC as Vref
       ADLAR = 0   ? right-adjust result
       MUX[3:0] = 0010 ? ADC2 (default channel for this lab)
    */
    ADMUX = (1 << REFS0) | (0 << REFS1)     // AVCC reference
          | (0 << ADLAR)
          | (2 << MUX0);                    // ADC2 = 0b0010

    /* ADCSRB – no auto-trigger source, no AC multiplexer */
    ADCSRB = 0x00;

    /*  disable the digital input buffer
       on ADC2 to reduce power/noise coupling */
    DIDR0 |= (1 << ADC2D);

    /* ADCSRA – enable ADC, clear any pending flag, set prescaler
       ADEN=1 (enable)
       ADIF=1 (write 1 to clear)
       ADPS2:0 = 100 ? prescaler ÷16 ? 2 MHz / 16 = 125 kHz (inside 50–200 kHz)
       ADSC/ADATE/ADIE remain 0 (single conversion, polling)
    */
    ADCSRA = (1 << ADEN) | (1 << ADIF) | (1 << ADPS2);
}

uint16_t adc_read(uint8_t channel)
{
	// 1) Select channel 0..7 without touching REFS/ADLAR
	channel &= 0x0F;                   // limit to 0..15 (328P uses 0..7)
	ADMUX = (ADMUX & 0xF0) | channel;  // update MUX[3:0] only

	// (Optional) Disable digital buffer for this channel (saves power/noise)
	// DIDR0 |= (1 << channel);  // safe when channel 0..5; ADC6/7 are analog-only

	// 2) Start conversion
	ADCSRA |= (1 << ADSC);

	// 3) Poll until complete
	// Option A: wait for ADIF to set
	while ( (ADCSRA & (1 << ADIF)) == 0 ) { /* wait */ }
	// or Option B: while (ADCSRA & (1<<ADSC));  // when ADSC goes 0, done

	// Clear ADIF by writing 1 (datasheet requirement)
	ADCSRA |= (1 << ADIF);

	// 4) Read result (right-adjusted): read ADCL FIRST, then ADCH
	uint16_t result = ADCL;
	result |= ((uint16_t)ADCH) << 8;
	
	return result;   // 10-bit value in bits [9:0], range 0..1023
}

uint16_t adc_convert_mv(uint16_t value)
{
	// Reference voltage in mV (5.00 V = 5000 mV)
	const uint16_t VREF_MV = 5000;

	// 10-bit ADC ? 1024 steps
	// Scale: (value * 5000) / 1024
	uint32_t result = (uint32_t)value * VREF_MV;
	result /= 1024;

	return (uint16_t)result;  // return in mV
}