/*
 * Lab2.c
 *
 * Created: 8/5/2025 4:01:58 AM
 * Author : harold
 */ 


#define F_CPU 2000000UL // 2MHz system clock

#include <avr/io.h>
#include <util/delay.h>

// Initialization function from Q4.3
void usart_init(uint16_t ubrr) {
	// Set the baud rate
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;

	// Enable the transmitter
	UCSR0B = (1 << TXEN0);

	// Set frame format: 8 data bits, 1 stop bit
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Transmission function from Q4.4
void usart_transmit(uint8_t data) {
	// Wait for the transmit buffer to be empty
	while (!(UCSR0A & (1 << UDRE0))) {
		// Do nothing, just wait
	}
	// Load the data into the UDR0 register to send it
	UDR0 = data;
}



int main(void)
{
    /* Create an array to store prime numbers up to 300 */
    uint16_t prime_numbers[62];  // There are 62 prime numbers up to 300
    uint16_t prime_count = 0;
    uint16_t num, i;
    uint16_t is_prime;
    
    /* Find all prime numbers up to 300 */
    for (num = 2; num <= 300; num++) {
        is_prime = 1;  // Assume number is prime initially
        
        /* Check if num can be divided by any number smaller than it */
        for (i = 2; i < num; i++) {
            if (num % i == 0) {
                is_prime = 0;  // Number is not prime
                break;
            }
        }
        
        /* If number is prime, add it to the array */
        if (is_prime) {
            prime_numbers[prime_count] = num;
            prime_count++;
        }
    }
    
    //  Call usart_init() using the UBRR value calculated in Q2.2, which is 12.
    usart_init(12);
	
	// Define the number to send


    //  Start an infinite loop
    while (1) {
        //  Set up a for-loop to iterate through the entire primes array.
        for (int i = 0; i < 62; i++) {
	        uint16_t current_prime = prime_numbers[i];
	        uint8_t hundreds, tens, ones;

	        // a. Extract the individual characters of each prime number.
	        // All primes are formatted as 3-digit numbers per the lab manual.
	        hundreds = current_prime / 100;
	        tens = (current_prime / 10) % 10;
	        ones = current_prime % 10;

	        // b. Call usart_transmit() as needed for each digit.
	        usart_transmit(hundreds + '0');
	        usart_transmit(tens + '0');
	        usart_transmit(ones + '0');

	        // c. Call usart_transmit() for the comma and space as needed.
	        usart_transmit(',');
	        usart_transmit(' ');
	        
	        // Add a small delay to make the output readable in the terminal
	        _delay_ms(100);
        }
    }
}

