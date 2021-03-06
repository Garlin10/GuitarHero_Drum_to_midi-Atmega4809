/*
  debounce.h. Snigelens version of Peter Dannegger's debounce routines.
  Debounce up to eight buttons on one port.  $Rev: 577 $
 */
#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// Buttons connected to PA0 and PA1

#define E2_PORT  PORTE.IN
#define E2_DIR	 PORTE.DIR
#define E1_PORT  PORTE.IN
#define E1_DIR	 PORTE.DIR
#define E0_PORT  PORTE.IN
#define E0_DIR	 PORTE.DIR
#define B1_PORT  PORTB.IN
#define B1_DIR	 PORTB.DIR
#define Bal_PIN   2
#define E1_PIN	  1
#define E0_PIN	  0
#define B1_PIN	  1

#define BUTTON1_MASK (1<<Bal_PIN)
#define BUTTON2_MASK (1<<E1_PIN)
#define BUTTON3_MASK (1<<E0_PIN)
#define BUTTON4_MASK (1<<B1_PIN)
#define BUTTON_MASK  (BUTTON1_MASK)

typedef enum 
{ 
	Released,
    PushedDown_Wait, 
    PushedDown_DoingSomething, 
	Released_Wait 
} Button_Machine;




// Variable to tell that the button is pressed (and debounced).
// Can be read with button_down() which will clear it.
extern volatile uint8_t buttons_down;

// Return non-zero if a button matching mask is pressed.
uint8_t button_down(uint8_t button_mask, char port);

// Make button pins inputs and activate internal pullups.
void debounce_init(void);

// Decrease 2 bit vertical counter where mask = 1.
// Set counters to binary 11 where mask = 0.
#define VC_DEC_OR_SET(high, low, mask)		\
    low = ~(low & mask);			\
    high = low ^ (high & mask)

// Check button state and set bits in the button_down variable if a
// debounced button down press is detected.
// Call this function every 10 ms or so.
static inline void debounce(void)
{
    // Eight vertical two bit counters for number of equal states
    static uint8_t vcount_low  = 0xFF, vcount_high = 0xFF;
    // Keeps track of current (debounced) state
    static uint8_t button_state = 0;

    // Read buttons (active low so invert with ~). Xor with
    // button_state to see which ones are about to change state
    uint8_t state_changed = ~PORTE_IN ^ button_state;

    // Decrease counters where state_changed = 1, set the others to 0b11.
    VC_DEC_OR_SET(vcount_high, vcount_low, state_changed);
    
    // Update state_changed to have a 1 only if the counter overflowed
    state_changed &= vcount_low & vcount_high;
    // Change button_state for the buttons who's counters rolled over
    button_state ^= state_changed;

    // Update button_down with buttons who's counters rolled over
    // and who's state is 1 (pressed)
    buttons_down |= button_state & state_changed;
}
#endif
