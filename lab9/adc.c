#include <avr/io.h>

#include "adc.h"


void adc_init(void)
{
    // Initialize the ADC
    // Use the AVCC high voltage reference
    ADMUX &= ~(1 << REFS1);
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Use prescalar 128 to fit in the range 50kHz to 200kHz
    ADMUX |= (1 << ADLAR); // Use 8 bits instead of 10
    ADCSRA |= (1 << ADEN); // Enable the ADC
}

unsigned char adc_sample(unsigned char channel)
{
    // Set ADC input mux bits to 'channel' value
    // Copy lower 4 bits
    unsigned char mask = 0x0f;
    ADMUX &= ~mask;
    ADMUX |= (channel & mask);

    // Convert an analog input and return the 8-bit result
    ADCSRA |= (1 << ADSC); // Start a sample
    while ( (ADCSRA & (1 << ADSC)) != 0 ){} // Wait until ADC is done
    return ADCH; // Once the ADC is done, collect the result

}
