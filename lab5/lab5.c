/********************************************
*
*  Name: Muyang Ye
*  Email: muyangye@usc.edu
*  Section: 31007R
*  Assignment: Lab 5 - Traffic light
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "lcd.h"

// S0 = Main Street Green
// S1 = Continuable Small
// S2 = Continuable Left
// S3 = Must Return Small
// s4 = Must Return Left
enum states { S0, S1, S2, S3, S4 };

int main(void) {

    /* Call lcd_init to set up the LCD */
    lcd_init();

    /* Set up I/O ports for buttons */
    // Enable pull-up resistors
    PORTC |= 0x1e;

    /* Call lcd_stringout to print out your name */
    // Clear the screen
    lcd_writecommand(1);
    lcd_stringout("Muyang Ye");

    /* Use snprintf to create a string with your birthdate */
    // Initializes to 17 because the max size of a row in LCD is 16
    char birthday[17];
    char month = 10;
    char day = 30;
    char year = 02;
    snprintf(birthday, 17, "%d/%d/%02d", month, day, year);

    /* Use lcd_moveto to start at an appropriate column 
       in the bottom row to appear centered */
    lcd_moveto(1, 4);

    /* Use lcd_stringout to print the birthdate string */
    lcd_stringout(birthday);

    /* Delay 1 second */
    _delay_ms(1000);

    // Clear the screen
    lcd_writecommand(1);

    /* Initialize State machine variables */
    char curState = S0;
    char main = 'G';
    char left = 'R';
    char small = 'R';
    char outputMain[7];
    char outputSmall[8];
    char outputLeft[7];
    char count = 0;
    char L1 = 0;
    char L2 = 0;
    // Can't use S1 and S2 as button names because already defined as state names. L3 = S1 and L4 = S2
    char L3 = 0;
    char L4 = 0;

    while(1)
    { 
	    /* Read the sensors */
        // The below if statements check to display which 'L' or 'S'
        lcd_moveto(1, 8);
        if ( (PINC & (1 << 1)) == 0 )
        {
            lcd_stringout("L");
            L1 = 1;
        } else {
            // If not pressed, output an empty string. Can't seperate lcd_stringout out because of different location
            L1 = 0;
            lcd_stringout(" ");
        }
        lcd_moveto(1, 9);
        if ( (PINC & (1 << 2)) == 0 )
        {
            L2 = 1;
            lcd_stringout("L");
        } else {
            lcd_stringout(" ");
            L2 = 0;
        }
        lcd_moveto(1, 10);
        if ( (PINC & (1 << 3)) == 0 )
        {
            L3 = 1;
            lcd_stringout("S");
        } else {
            L3 = 0;
            lcd_stringout(" ");
        }
        lcd_moveto(1, 11);
        if ( (PINC & (1 << 4)) == 0 )
        {
            L4 = 1;
            lcd_stringout("S");
        } else {
            L4 = 0;
            lcd_stringout(" ");
        }

        /* Determine new state */
        if (curState == S0)
        {
            // First L1 because when both buttons are pressed, first go to left green then small green
            // If L1 or L2 is pressed and have stayed on main street for >= 2 seconds
            if ( (L1 || L2) && count >= 100 )
            {
                left = 'G';
                main = 'R';
                curState = S2;
                count = 0;
            }

            // If S1 or S2 is pressed and have stayed on main street for >= 2 seconds
            if ( (L3 || L4) && count >= 100 )
            {
                // Change to small green
                small = 'G';
                main = 'R';
                curState = S1;
                count = 0;
            }
        }

        else if (curState == S1)
        {
            // If has stayed on small street for 1 second
            if (count == 50)
            {
                // If L1 or L2 is pressed
                if ( L1 || L2 )
                {
                    // Change to left green
                    left = 'G';
                    small = 'R';
                    curState = S4;
                    count = 0;
                }

                // Otherwise it is time to change to main green
                else
                {
                    small = 'R';
                    main = 'G';
                    curState = S0;
                    count = 0;
                }
            }
        }
        
        else if (curState == S2)
        {
            // If has stayed on left street for 1 second
            if (count == 50)
            {
                // If S1 or S2 is pressed
                if ( L3 || L4 )
                {
                    // Change to small green
                    left = 'R';
                    small = 'G';
                    curState = S3;
                    count = 0;
                }

                // Otherwise it is time to change to main green
                else
                {
                    left = 'R';
                    main = 'G';
                    curState = S0;
                    count = 0;
                }
            }
        }
        
        // If left green is just before this small green and 1 second has passed
        if (curState == S3 && count == 50)
        {
            // Change directly to main green
            small = 'R';
            main = 'G';
            curState = S0;
            count = 0;
        }
        
        // If small green is just before this left green and 1 second has passed
        if (curState == S4 && count == 50)
        {
            // Change directly to main green
            left = 'R';
            main = 'G';
            curState = S0;
            count = 0;
        }

	    /* Show outputs */
        // This %50 == 0 is to speed up button reaction
        if (count % 50 == 0){
            // Combine the string to be shown
            snprintf(outputMain, 7, "Main=%c", main);
            snprintf(outputSmall, 8, "Small=%c", small);
            snprintf(outputLeft, 7, "Left=%c", left);
            // Move to corresponding cursor location and output
            lcd_moveto(0, 0);
            lcd_stringout(outputMain);
            lcd_moveto(0, 8);
            lcd_stringout(outputSmall);
            lcd_moveto(1, 0);
            lcd_stringout(outputLeft);
        }
        
        // For debug
        lcd_moveto(1, 13);
        char test[2];
        snprintf(test, 2, "%d", curState);
        lcd_stringout(test);
        
        // Delay only 20ms and have a count so that we can implement different rates for check and output
        _delay_ms(20);
        count += 1;
    }

    return 0;   /* never reached */
}
