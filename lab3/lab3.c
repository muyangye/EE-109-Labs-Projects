/********************************************
 *
 *  Name: Muyang Ye
 *  Email: muyangye@usc.edu
 *  Lab section: 31007
 *  Assignment:
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
// #include <avr/iom328p.h>

#define DOT_LENGTH 250 /* Define the length of a "dot" time in msec */

void dot(void);
void dash(void);
void makeOutput(char);
char checkInput(char);

int main(void) {

  // Initialize appropriate DDR registers
  DDRD |= 0x04;

  // Initialize the LED output to 0

  // Enable the pull-up resistors for the
  // 3 button inputs
  PORTB |= 0x38;
  // this delay is to prevent the LED from flashing at the very first
  _delay_ms(200);

  // Loop forever
  while (1) {

    //  Use if statements and the checkInput()
    //  function to determine if a button
    //  is being pressed and then output
    //  the correct dot/dash sequence by
    //  calling the dot(), dash(), and
    //  using appropriate delay functions

    // U
    if (checkInput(3)) {
      dot();
      dot();
      dash();
      // delay dash length after a letter
      _delay_ms(3 * DOT_LENGTH);
    }
    // S
    if (checkInput(4)) {
      dot();
      dot();
      dot();
      _delay_ms(3 * DOT_LENGTH);
    }
    // C
    if (checkInput(5)) {
      dash();
      dot();
      dash();
      dot();
      _delay_ms(3 * DOT_LENGTH);
    }
    
  }

  return 0; /* never reached */
}

/*
  dot() - Makes the output LED blink a "dot".

  Write code to generate a dot by using the makeOutput function
  to turn the output ON and OFF with appropriate delays.
  Be sure you don't forget to also delay an
  appropriate time after you turn the output off
*/
void dot() {
  makeOutput(1);
  _delay_ms(DOT_LENGTH);
  makeOutput(0);
  // delay dot length within a letter
  _delay_ms(DOT_LENGTH);
}

/*
  dash() - Makes the output LED blink a "dash".

  Write code to generate a dash by using the makeOutput function
  to turn the output ON and OFF with appropriate delays.
  Be sure you don't forget to also delay
  appropriate time after you turn the output off
*/
void dash() {
  makeOutput(1);
  // dashes' length is 3 times dots' length
  _delay_ms(3 * DOT_LENGTH);
  makeOutput(0);
  _delay_ms(DOT_LENGTH);
}

/*
  makeOutput() - Changes the output bit to either a zero
  or one, based on the input argument "value".

  If the argument is zero, turn the output OFF,
  otherwise turn the output ON.

  Do not use any delays here.  Just use bit-wise operations
  to make the appropriate PORT bit turn on or off.
*/
void makeOutput(char value) {
  if (value == 0) {
    // set PORTD bit 2 to 0
    PORTD &= ~(1 << 2);
  } else {
    // set PORTD bit 2 to 1(output)
    PORTD |= (1 << 2);
  }
}

/*
  checkInput(bit) - Checks the state of the input bit specified by the
  "bit" argument (0-7), and returns either 0 or 1 depending on its state.

  Write code to use the appropriate group's PIN register and determine if
  the specified bit (which is passed as the argument) of that group is
  pressed or not.  Think carefully about what bit value means "pressed"
  when connected to a pushbutton.
 */
// 0 means "pressed"
char checkInput(char bit) { return ((PINB & (1 << bit)) == 0); }
