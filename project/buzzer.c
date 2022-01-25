#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

#include "buzzer.h"

/*
    variable_delay_us - Delay a variable number of microseconds
*/
void variable_delay_us(int delay)
{
    int i = (delay + 5) / 10;

    while (i--)
        _delay_us(10);
}

/*
  Play a tone at the frequency specified for one second
*/
void play_note(unsigned short freq)
{
    unsigned long period;

    period = 1000000 / freq;      // Period of note in microseconds

    while (freq--) {
	    // Make PC5 high
	    PORTC |= (1 << 5);

	    // Use variable_delay_us to delay for half the period
		variable_delay_us(period / 2);

	    // Make PC5 low
		PORTC &= ~(1 << 5);

	    // Delay for half the period again
		variable_delay_us(period / 2);
    }
}