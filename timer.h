#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <stdint.h>


// Function declarations
void timer0_init(void);
void timer1_init(void);
void timer1_start(void);
void timer1_stop(void);
void timer1_clear_compare_match_b_flag(void);
#endif // TIMER_H
