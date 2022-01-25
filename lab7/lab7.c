/********************************************
 *
 *  Name: Muyang Ye
 *  Email: muyangye@usc.edu
 *  Section: 31007R
 *  Assignment: Lab 7 - Rotary Encoder
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"

void variable_delay_us(int);
void play_note(unsigned short);

// Frequencies for natural notes from middle C (C4)
// up one octave to C5.
unsigned short frequency[8] =
    { 262, 294, 330, 349, 392, 440, 494, 523 };

// Set to volatile to allow changes of these variables outside the main function
volatile unsigned char new_state, old_state;
volatile unsigned char changed = 0;  // Flag for state change
volatile int count = 0;		// Count to display
volatile unsigned char a, b;
volatile unsigned char input;

int main(void) {

    // Initialize DDR and PORT registers and LCD
	lcd_init(); // Initialize LCD
	DDRB |= (1 << 4); // Set PORTB bit 4 to output
	PORTC |= (1 << 1) | (1 << 5); // Enable pull-up resistors
	PCICR |= (1 << PCIE1); // Enable Port C interrupts
	PCMSK1 |= (1 << PCINT9) | (1 << PCINT13); // Listen to PC1 and PC5
	sei(); // Enable global interrupts

    // Write a spash screen to the LCD
	lcd_writecommand(1); // Clear the screen
	lcd_moveto(0, 2);
    lcd_stringout("EE109 Lab 7");
    lcd_moveto(1, 3);
    lcd_stringout("Muyang Ye");
    _delay_ms(1000);
    lcd_writecommand(1); // Clear the screen

    // Read the A and B inputs to determine the intial state.
    // In the state number, B is the MSB and A is the LSB.
    // Warning: Do NOT read A and B separately.  You should read BOTH inputs
    // at the same time, then determine the A and B values from that value.    

    if (!b && !a)
	old_state = 0;
    else if (!b && a)
	old_state = 1;
    else if (b && !a)
	old_state = 2;
    else
	old_state = 3;

    new_state = old_state;

    while (1) {                 // Loop forever
        if (changed) { // Did state change?
	        changed = 0;        // Reset changed flag

	        // Output count to LCD
			char buf[17]; // Allocate 17 spots because the max size of a row in LCD is 16
			snprintf(buf, 17, "%04d", count); // %04d to allow -999 to 9999
			lcd_moveto(0, 0);
			lcd_stringout(buf);

	        // Do we play a note?
	        if ((count % 8) == 0) {

		    // Determine which note (0-7) to play
			unsigned char index = abs(count) % 64 / 8;

		    // Find the frequency of the note
			unsigned short freq = frequency[index];

		    // Call play_note and pass it the frequency
			play_note(freq);
	        }
        }
    }
}

/*
  Play a tone at the frequency specified for one second
*/
void play_note(unsigned short freq)
{
    unsigned long period;

    period = 1000000 / freq;      // Period of note in microseconds

    while (freq--) {
	    // Make PB4 high
	    PORTB |= (1 << 4);

	    // Use variable_delay_us to delay for half the period
		variable_delay_us(period / 2);

	    // Make PB4 low
		PORTB &= ~(1 << 4);

	    // Delay for half the period again
		variable_delay_us(period / 2);
    }
}

/*
    variable_delay_us - Delay a variable number of microseconds
*/
void variable_delay_us(int delay)
{
    int i = (delay + 5) / 10;

    while (i--)
        _delay_us(10);
}

ISR(PCINT1_vect)
{
	// Read the input bits and determine A and B.
	input = PINC & ((1 << 1) | (1 << 5)); // Temporarily stores a and b
	a = input & (1 << 1);
	b = input & (1 << 5);
	// For each state, examine the two input bits to see if state
	// has changed, and if so set "new_state" to the new state,
	// and adjust the count value.
	if (old_state == 0) {
	    // Handle A and B inputs for state 0
		if (a) {
			new_state = 1;
			count += 1;
		} else if (b) {
			new_state = 2;
			count -= 1;
		}
	}
	else if (old_state == 1) {
	    // Handle A and B inputs for state 1
		if (!a) {
			new_state = 0;
			count -= 1;
		} else if (b) {
			new_state = 3;
			count += 1;
		}
	}
	else if (old_state == 2) {
	    // Handle A and B inputs for state 2
		if (a) {
			new_state = 3;
			count -= 1;
		} else if (!b) {
			new_state = 0;
			count += 1;
		}
	}
	else {   // old_state = 3
	    // Handle A and B inputs for state 3
		if (!a) {
			new_state = 2;
				count += 1;
		} else if (!b) {
			new_state = 1;
			count -= 1;
		}
	}

	// If state changed, update the value of old_state,
	// and set a flag that the state has changed.
	if (new_state != old_state) {
	    changed = 1;
	    old_state = new_state;
	}
}