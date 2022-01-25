/********************************************
 *
 *  Name: Muyang Ye
 *  Email: muyangye@usc.edu
 *  Section: 31007
 *  Assignment: Lab 9 - Serial Communications
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "adc.h"
#include "check9.h"

volatile unsigned char flag = 0; // This flag checks if the received message is complete
volatile unsigned char i = 0; // This index tracks the current number of chars received
volatile char buffer[17]; // The buffer to store received string

// Serial communications functions and variables
void serial_init(unsigned short);
void serial_stringout(char *);
void serial_txchar(char);

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0 register

// ADC functions and variables

#define ADC_CHAN 0              // Buttons use ADC channel 0

char *messages[] = {
    "Hello           ",
    "How are you?    ",
    // Add more messgages
    "Is EE109 funny? ",
    "Noooo!          ",
    "Mark Redekopp?  ",
    "Yesss!          ",
    "Thank you       ",
    "What did you eat",
    "French Fries    ",
    "Good Bye        "
};

int main(void) {

    // Initialize the LCD, ADC and serial modules
    lcd_init();
    adc_init();
    serial_init(MYUBRR);
    
    // Initialize the baud rate checker
    check9_init();
 
    sei(); // Enable interrupts
    UCSR0B |= (1 << RXCIE0); // Enable receiver interrupts

    // Show the splash screen
    lcd_writecommand(1); // Clear the screen
    lcd_moveto(0, 2);
    lcd_stringout("EE109 Lab 9");
    lcd_moveto(1, 3);
    lcd_stringout("Muyang Ye");
    _delay_ms(1000);
    lcd_writecommand(1); // Clear the screen
    // Display initial message
    lcd_moveto(0, 0);
    lcd_stringout("Select message  ");
    unsigned char sample;
    unsigned char index = 0;

    while (1) {                 // Loop forever

	    // Get an ADC sample to read buttons
        sample = adc_sample(0);

        // Up button pressed?
        if (sample >= 50 && sample <= 60) // Up is about 55
        {
            _delay_ms(200); // Deboucing delay
            index = index-1 < 0 ? 9 : index-1; // If index < 0, wrap around to 9
            lcd_moveto(0, 0);
            lcd_stringout(messages[index]);
        }

        // Down button pressed?
        if (sample >= 100 && sample <= 110) // Down is about 104
        {
            _delay_ms(200); // Deboucing delay
            index = index+1 > 9 ? 0 : index+1; // If index > 9, wrap around to 0
            lcd_moveto(0, 0);
            lcd_stringout(messages[index]);
        }

        // Select button pressed?
        if (sample >= 200 && sample <= 215) // Select is about 208
        {
            _delay_ms(200); // Deboucing delay
            serial_stringout(messages[index]);
        }
        
        // Message received from remote device?
        if (flag == 1)
        {
            // If so, reset flag and output the received message on second line
            flag = 0;
            lcd_moveto(1, 0);
            lcd_stringout(buffer);
        }
    }
}

/* ----------------------------------------------------------------------- */

void serial_init(unsigned short ubrr_value)
{

    // Set up USART0 registers
    UBRR0 = ubrr_value; // Set baud rate
    UCSR0C = (3 << UCSZ00);
    UCSR0B |= (1 << TXEN0 | 1 << RXEN0);
    // Enable tri-state buffer
    DDRD |= (1 << 3);
    PORTD &= ~(1 << 3);

}

void serial_txchar(char ch)
{
    while ((UCSR0A & (1 << UDRE0)) == 0);
    UDR0 = ch;
}

void serial_stringout(char *s)
{

    // Call serial_txchar in loop to send a string
    // (Very similar to lcd_stringout in lcd.c)
    int j = 0;
    while (s[j] != '\0')
    {
        serial_txchar(s[j]);
        j += 1;
    }

}

ISR(USART_RX_vect)
{

    // Handle received character
    char ch;
    ch = UDR0; // Get the received character

    buffer[i] = ch; // Store in buffer
    i += 1;

    // If message is complete, set flag and reset index
    if (i == 16)
    {
        i = 0;
        flag = 1;
    }

}
