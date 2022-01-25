/********************************************
*
*  Name:
*  Email:
*  Section:
*  Assignment: Lab 4 - Up/Down counter on LCD display
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

enum states { UP, DOWN };

int main(void) {

    // Setup DDR and PORT bits for the 2 input buttons as necessary
    // no need to set DDRC because it is automatically set up to 0, only enable the pull-up resistors of PC2 and PC 4
    PORTC |= 0x14;

    // Initialize the LCD
    lcd_init();


    // Use a state machine approach to organize your code
    //   - Declare and initialize a variable to track what state you
    //     are in by assigning the values UP or DOWN to that variable.
    char curState = UP;
    char curCount = 0;
    char counter = 0;

    while (1) {               // Loop forever
        // Use an outer if statement to select what state you are in
        // Then inside each 'if' or 'else' block, perform the desired
        // output operations and then sample the inputs to update
        // the state for the next iteration
        if (curState == UP)
        {
            // check if PC4 is 0. i.e. PC4 is pressed
            if ((PINC & 0x10) == 0)
            {
                curState = DOWN;
                // only when counter reaches 100 curCount would -/+ so that 500ms flash but 5ms check
                curCount -= counter/100;
            }
            else curCount += counter/100;
        }
        else
        {
            // check if PC2 is 0. i.e. PC2 is pressed
            if ((PINC & 0x04) == 0)
            {
                curState = UP;
                curCount += counter/100;
            }
            else curCount -= counter/100;
        }
        // if the number is greater than 9 or less than 0, LCD shield is unable to display in one digit
        if (curCount > 9) curCount = 0;
        if (curCount < 0) curCount = 9;
        // using the counter to increase the responsiveness of the button press
        if (counter == 100)
        {
            // to clear the screen
            lcd_writecommand(1);
            // to display ASCII version
            lcd_writedata('0' + curCount);
            // to reset counter
            counter = 0;
        }
        
        // Delay before we go to the next iteration of the loop
        _delay_ms(5);
        counter += 1;
    }

    return 0;   /* never reached */
}

