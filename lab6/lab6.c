/********************************************
 *
 *  Name:
 *  Email:
 *  Section:
 *  Assignment: Lab 6 - Analog-to-digital conversion
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "lcd.h"
#include "adc.h"

void rand_init(void);

int main(void)
{

    // Initialize the LCD
    lcd_init();

    // Initialize the ADC
    adc_init();

    // Initialize the random number function
    rand_init();

    // Write splash screen
    lcd_writecommand(1); // Clear the screen
    lcd_moveto(0, 2);
    lcd_stringout("EE109 Lab 6");
    lcd_moveto(1, 3);
    lcd_stringout("Muyang Ye");
    _delay_ms(1000);
    lcd_writecommand(1); // Clear the screen

    // Find a random initial position of the 'X'
    unsigned char btnPos = rand() % 16;

    // Display the 'X' on the screen
    lcd_moveto(0, btnPos);
    lcd_stringout("X");

    // Inital position of 'X' is determined by the initial value of potentiometer
    unsigned char potRes = adc_sample(3);
    unsigned char potPos = potRes / 16; // 255/16=15.9375, so it will never go over 15 (the maximum size of a row of LCD) and must be >= 0
    lcd_moveto(1, potPos);
    lcd_stringout("^");

    int count = 0; // Used to detect whether '^' is aligned with 'X' for 2 seconds
    while (1) {                 // Loop forever
	// Check for button pressed
    unsigned char btnRes = adc_sample(0); // Sample the button's analog signal
        // If pressed, move 'X' to new position
        if (btnRes >= 150 && btnRes <= 160) // Left button is pressed
        {
            // Erase the original 'X'
            lcd_moveto(0, btnPos);
            lcd_stringout(" ");
            if (btnPos >= 1) btnPos -= 1; // To avoid 'X' going beyond the screen
            // Write the new 'X'
            lcd_moveto(0, btnPos);
            lcd_stringout("X");
            _delay_ms(100);
        }
        else if (btnRes >= 0 && btnRes <= 10) // Right button is pressed
        {
            // Erase the original 'X'
            lcd_moveto(0, btnPos);
            lcd_stringout(" ");
            if (btnPos <= 14) btnPos += 1; // To avoid 'X' going beyond the screen
            // Write the new 'X'
            lcd_moveto(0, btnPos);
            lcd_stringout("X");
            _delay_ms(100);
        }


        // Do a conversion of potentiometer input
        potRes = adc_sample(3); // Sample the potentiometer's analog signal


	// Move '^' to new position
    if (potRes / 16 != potPos)
    {
        // Erase the original '^'
        lcd_moveto(1, potPos);
        lcd_stringout(" ");
        potPos = potRes / 16; // 255/16=15.9375, so it will never go over 15 (the maximum size of a row of LCD) and must be >= 0
        // Write the new '^'
        lcd_moveto(1, potPos);
        lcd_stringout("^");
    }

	// Delay
    _delay_ms(10);

	// Check if '^' is aligned with 'X'
    if (btnPos == potPos)
    {
        count ++;
        if (count == 200) // If '^' is aligned with 'X' for 2 seconds
        {
            // Write success message and stop the game
            lcd_writecommand(1); // Clear the screen
            lcd_moveto(0, 0);
            lcd_stringout("You Win!");
            break;
        }
    }
    else count = 0; // If '^' is aligned with 'X' but for <2 seconds and then diverges, reset the time

    }

    return 0;   /* never reached */
}

void rand_init()
{
    int seed = 0;
    unsigned char i, j, x;

    // Build a 15-bit number from the LSBs of an ADC
    // conversion of the channels 1-5, 3 times each
    for (i = 0; i < 3; i++) {
	for (j = 1; j < 6; j++) {
	    x = adc_sample(j);
	    x &= 1;	// Get the LSB of the result
	    seed = (seed << 1) + x; // Build up the 15-bit result
	}
    }
    srand(seed);	// Seed the rand function
}
