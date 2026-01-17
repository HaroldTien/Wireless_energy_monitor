/*
 * led.c
 *
 * Created: 19/09/2025 5:55:04 PM
 *  Author: thar970
 */ 

#include "led.h"
#include <avr/io.h>

// Choose the LED pin (update if your Proteus LED is on a different pin)
#define LED_DDR   DDRB
#define LED_PORT  PORTB
#define LED_PINN  PORTB5

void led_init(void){
	LED_DDR  |= (1 << LED_PINN);  // output
	LED_PORT &= ~(1 << LED_PINN); // start low (LED off for active-high wiring)
}

void led_toggle(void){
	LED_PORT ^= (1 << LED_PINN);
}
