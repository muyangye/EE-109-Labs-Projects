#include <avr/io.h>
#include <stdlib.h>

#include "timer.h"

void timer_init()
{
    // Because the range finder can go from 2cm to 4m, the time interval can be 400cm * 58 = 23200us = 0.0232s
    TCCR1B |= (1 << WGM12); // Set to CTC mode
    TIMSK1 |= (1 << OCIE1A); // Enable timer1 interrupt
    OCR1A = 46400; // Load the MAX count, assuming prescalar = 8. 16MHz * 0.0232s / 8 = 46400
    TCCR2A |= (1 << WGM21) | (1 << WGM20) | (1 << COM2A1); // PWM initializations from PWM slides
    TCCR2B |= (1 << CS22); // Set timer2's prescalar to 256
    OCR2A = 255; // Load the MAX count 255
    TCCR0A |= (1 << WGM01); // Set to 
    TIMSK0 |= (1 << OCIE0A); // Enable timer2 interrupt
    OCR0A = 120; // Load the frequency 120
}