#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "adc.h"

volatile unsigned char MSB = 0; // MSB is 10s
volatile unsigned char SEC = 0; // SEC is 1s
volatile unsigned char LSB = 0; // LSB is 0.1s

void timer_init()
{
    TCCR1B |= (1 << WGM12); // Set to CTC mode
    TIMSK1 |= (1 << OCIE1A); // Enable Timer Interrupt
    OCR1A = 25000; // Load the MAX count, assuming prescalar = 64. 16MHz * 0.1s / 64 = 25000
    sei(); // Enable global interrupts
}

int main(void)
{
    lcd_init(); // Initialize the LCD
    adc_init(); // Initialize the ADC
    timer_init(); // Initialize the timer
    unsigned char state = 0; // 0 is Stopped, 1 is Started, 2 is Lapped
    unsigned char sample; // Stores the adc sample 
    char time[17]; // Allocate 17 spots because the max size of a row in LCD is 16

    // Write splash screen
    lcd_writecommand(1); // Clear the screen
    lcd_moveto(0, 2);
    lcd_stringout("EE109 Lab 8");
    lcd_moveto(1, 3);
    lcd_stringout("Muyang Ye");
    _delay_ms(1000);
    lcd_writecommand(1); // Clear the screen
    lcd_stringout("00.0");
    unsigned char flag = 0;

    while (1)
    {
        sample = adc_sample(0);
        if (state == 0) // Stopped
        {
            if (sample >= 50 && sample <= 60)
            {
                state = 1;
                TCCR1B |= (1 << CS11) | (1 << CS10); // Start the internal clock with prescalar = 64
                _delay_ms(100); // Debouncing delay
            }
            else if (sample >= 100 && sample <= 110) // Lap_Reset button is pressed
            {
                // Clear the time
                MSB = 0;
                SEC = 0;
                LSB = 0;
                flag = 1; // Only need to change the display when in Started state or clearing the time (in Stopped click Lap_Reset button)
                _delay_ms(100); // Debouncing delay
            }
        }
        else if (state == 1) // Started
        {
            flag = 1; // Only need to change the display when in Started or clearing the time (in Stopped click Lap_Reset button)
            if (sample >= 50 && sample <= 60)
            {
                state = 0;
                TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // When go to Stopped, stop the internal clock
                _delay_ms(100); // Debouncing delay
            }
            else if (sample >= 100 && sample <= 110)
            {
                state = 2;
                _delay_ms(100); // Debouncing delay
            }
        }
        else if (state == 2) // Lapped
        {
            if (sample >= 50 && sample <= 60 || sample >= 100 && sample <= 110) // Lapped can only go to Started regardless of which button is pressed
            {
                state = 1;
                _delay_ms(100); // Debouncing delay
            }
        }
        if (flag) // If there's a need to change display
        {
            flag = 0; // Reset the flag
            snprintf(time, 17, "%d%d.%d", MSB, SEC, LSB); // Formulate the display string
            lcd_moveto(0, 0);
            lcd_stringout(time);
        }
    }

    return 0;   /* never reached */
}

ISR(TIMER1_COMPA_vect)
{
    LSB += 1; // No matter what, LSB increases by 1 because in ISR means 0.1s has elapsed
    if (LSB == 10) // If LSB goes to 10, it means 1 second has passed
    {
        SEC += 1;
        LSB = 0; // Reset LSB
    }
    if (SEC == 10) // If SEC goes to 10, it means 10 seconds has passed
    {
        MSB += 1;
        SEC = 0; // Reset SEC
    }
    if (MSB == 6) MSB = 0; // When MSB goes to 6, the other digits are already reset because they must go to 10 to change MSB. So only need to update MSB
}