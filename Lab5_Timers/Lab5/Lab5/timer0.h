/*
 * timer0.h
 *
 * Created: 19/09/2025 5:54:48 PM
 *  Author: thar970
 */ 

#ifndef TIMER0_H_
#define TIMER0_H_

#include <stdint.h>

/**
 * @brief Initializes Timer0 in Normal mode.
 */
void timer0_init(void);

/**
 * @brief Resets and starts Timer0 with a prescaler of 8.
 */
void timer0_start(void);

/**
 * @brief Stops Timer0 and returns the final count.
 * @return The 8-bit value from the TCNT0 register.
 */
uint8_t timer0_stop(void);

#endif /* TIMER0_H_ */