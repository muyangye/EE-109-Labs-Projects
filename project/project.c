#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "lcd.h"
#include "timer.h"
#include "buzzer.h"

volatile unsigned long pulseCount = 0; // Equals to TCNT1(the internal clock count value)
volatile unsigned char displayFlag = 0; // 0 means the current measurement is not finished, 1 means it is finished and we should output
volatile unsigned char overFlag = 0; // 0 means within range(400cm), 1 means out of range
volatile unsigned char thresholdFlag = 0; // 0 means we should adjust near threshold, 1 means we should adjust far threshold
// Below are rotary encoder
volatile unsigned char newState, oldState;
volatile unsigned char changed = 0;  // Flag for rotary encoder state change
volatile unsigned char a, b;
volatile unsigned char encoderInput;
volatile short encoderCount = 0; // encoder count to display

int main()
{
    sei(); // Enable global interrupts
    lcd_init(); // Initialize lcd
    timer_init(); // Initialize timer
    DDRD |= (1 << PD2); // Set rangefinder trigger to output
    // PORTD |= (1 << PD3); // Enable pull-up resistor for rangefinder trigger
    PORTB |= (1 << PB4); // Enable pull-up resistor for acquire button
    PCICR |= (1 << PCIE2); // Enable PORT D interrupts
    PCMSK2 |= (1 << PCINT19); // Listen to rangefinder echo (PD3)
    PORTC |= (1 << PC1) | (1 << PC2) | (1 << PC3); // Enable pull-up resistors for rotary encoder and threshold button
    PCICR |= (1 << PCIE1); // Enable PORT C interrupts
    PCMSK1 |= (1 << PCINT9) | (1 << PCINT10); // Enable rotary encoder interrupt
    DDRB |= (1 << PB3); // Set green LED to output
    DDRC |= (1 << PC4); // Set blue LED to output
    DDRC |= (1 << PC5); // Set buzzer to output

    // Show the splash screen
    lcd_writecommand(1); // Clear the screen
    lcd_moveto(0, 1);
    lcd_stringout("EE109 Project");
    lcd_moveto(1, 3);
    lcd_stringout("Muyang Ye");
    _delay_ms(1000);
    lcd_writecommand(1); // Clear the screen

    char distance[6]; // Buffer string for distance
    char near[9]; // Buffer string for near threshold
    char far[8]; // Buffer string for far threshold
    short nearDis = eeprom_read_word((void *) 100);
    short farDis = eeprom_read_word((void *) 200);
    // Make sure near and far range are 5cm apart when restarting and reading the eeprom memory
    if (farDis - 5 <= nearDis)
    {
        if (farDis - 5 < 1) nearDis = 1;
        else nearDis = farDis-5;
    }
    lcd_moveto(0, 0);
    snprintf(near, 9, "Near=%3d", nearDis);
    lcd_stringout(near);
    lcd_moveto(1, 0);
    snprintf(far, 8, "Far=%3d", farDis);
    lcd_stringout(far);

    // Rotary encoder code copied from lab7
    if (!b && !a)
	oldState = 0;
    else if (!b && a)
	oldState = 1;
    else if (b && !a)
	oldState = 2;
    else
	oldState = 3;
    newState = oldState;

    while (1)
    {
        if ( (PINB & (1 << PB4)) == 0 ) // Acquire button is pressed
        {
            PORTD |= (1 << PD2); // Start an measurement
            _delay_us(10);
            PORTD &= ~(1 << PD2); // Reset trigger
            _delay_ms(200); // Debouncing delay
        }
        if (displayFlag) // Measurement finished
        {
            lcd_moveto(0, 10);
            lcd_stringout("        ");// Clear the screen
            lcd_moveto(0, 9);
            if (overFlag) // If >400cm
            {
                lcd_stringout(">400.0");
                PORTC |= (1 << PC4);
                // PORTB &= ~(1 << PB3);
                overFlag = 0; // Reset overFlag
            }
            else
            {
                PORTC &= ~(1 << PC4);
                // Because we are not allowed to use floating point arithmetic, to get the decimal, first * 10 then % 10 to bring out the decimal
                unsigned int temp = pulseCount * 10 / 2 / 58; // Range formula from manufacturer's datasheet for the rangefinder
                unsigned int integer = temp / 10;
                unsigned int decimal = temp % 10;
                snprintf(distance, 6, "%2d.%d", integer, decimal);
                lcd_stringout(distance);
                if (integer > farDis) // Set to pure green if above far threshold
                {
                    OCR2A = 255;
                }
                else if (integer >= nearDis && integer <= farDis) // Color based on how far from near threshold/how near from far threshold
                {
                    OCR2A = 255 * (integer-nearDis) / (farDis-nearDis);
                }
                else
                {
                    OCR2A = 0; // Set to pure red if below near threshold
                    play_note(330);
                }
            }
            //// Reset displayFlag and internal clock count
            displayFlag = 0;
            pulseCount = 0;
        }
        if ( (PINC & (1 << PC3)) == 0) // Threshold button is pressed
        {
            if (!thresholdFlag) // Adjust near threshold
            {
                encoderCount = nearDis;
                thresholdFlag = 1;
                
            }
            else // Adjust far threshold
            {
                encoderCount = farDis;
                thresholdFlag = 0;
            }
            _delay_ms(200); // Debouncing delay
        }
        if (changed)
        {
            changed = 0;
            // Wrap around if exceed 400 or less than 1
            encoderCount = encoderCount > 400 ? 400 : encoderCount;
            encoderCount = encoderCount < 1 ? 1 : encoderCount;
            if (thresholdFlag) // Since we flip the threshold flag before, now 1 means we should output near range and 0 means far range
            {
                nearDis = encoderCount;
                // Make sure that near and far are at least 5cm apart
                if (farDis - 5 <= nearDis)
                {
                    if (farDis - 5 < 1) nearDis = 1;
                    else nearDis = farDis-5;
                }
                // Output near threshold
                lcd_moveto(0, 0);
                snprintf(near, 9, "Near=%3d", nearDis);
                lcd_stringout(near);
                eeprom_update_word((void *) 100, nearDis); // Only write to eeprom when we actually changed the range
            }
            else
            {
                farDis = encoderCount;
                if (nearDis + 5 >= farDis)
                {
                    if (nearDis + 5 > 400) farDis = 400;
                    else farDis = nearDis + 5;
                }
                // Output far threshold
                lcd_moveto(1, 0);
                snprintf(far, 8, "Far=%3d", farDis);
                lcd_stringout(far);
                eeprom_update_word((void *) 200, farDis); // Only write to eeprom when we actually changed the range
            }
        }
    }

    return 0; // Never reached
}

ISR(PCINT2_vect)
{
    if ( (PIND & (1 << PD3)) == 0 ) // Echo is done
    {
        pulseCount = TCNT1;
        TCCR1B &= ~(1 << CS11); // Clear the prescalar and stop the clock
        displayFlag = 1; // LCD should display the current measurement
    }
    else // Trigger is triggered
    {
        TCNT1 = 0;
        TCCR1B |= (1 << CS11); // Start the clock with prescalar = 8
    }
}

ISR(PCINT1_vect) // Rotary encoder code copied from lab7
{
	// Read the input bits and determine A and B.
	encoderInput = PINC & ((1 << PC1) | (1 << PC2)); // Temporarily stores a and b
	a = encoderInput & (1 << PC1);
	b = encoderInput & (1 << PC2);
	// For each state, examine the two input bits to see if state
	// has changed, and if so set "newState" to the new state,
	// and adjust the count value.
	if (oldState == 0) {
	    // Handle A and B inputs for state 0
		if (a) {
			newState = 1;
			encoderCount += 1;
		} else if (b) {
			newState = 2;
			encoderCount -= 1;
		}
	}
	else if (oldState == 1) {
	    // Handle A and B inputs for state 1
		if (!a) {
			newState = 0;
			encoderCount -= 1;
		} else if (b) {
			newState = 3;
			encoderCount += 1;
		}
	}
	else if (oldState == 2) {
	    // Handle A and B inputs for state 2
		if (a) {
			newState = 3;
			encoderCount -= 1;
		} else if (!b) {
			newState = 0;
			encoderCount += 1;
		}
	}
	else {   // oldState = 3
	    // Handle A and B inputs for state 3
		if (!a) {
			newState = 2;
				encoderCount += 1;
		} else if (!b) {
			newState = 1;
			encoderCount -= 1;
		}
	}

	// If state changed, update the value of oldState,
	// and set a flag that the state has changed.
	if (newState != oldState) {
	    changed = 1;
	    oldState = newState;
	}
}

ISR(TIMER1_COMPA_vect)
{
    pulseCount = OCR1A; // Something went wrong, set to maximum range
    TCCR1B &= ~(1 << CS11); // Clear the prescalar and stop the clock
    overFlag = 1; // Range is above 400cm
    displayFlag = 1; // LCD should also display >400 if range is above 400cm
}

ISR(TIMER0_COMPA_vect)
{
    
}