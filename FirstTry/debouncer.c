/*
  debounce.c
 */

#include "debouncer.h"

// Bits is set to one if a depounced press is detected.
volatile uint8_t buttons_down;

// Return non-zero if a button matching mask is pressed.
//button_mask-ot kivenni
//20ms várok utána váltok csak állapotot
uint8_t button_down(uint8_t button_mask)
{
    uint8_t PORT_INFO = PORTE.IN;
	PORT_INFO &= (button_mask);
	if(PORT_INFO)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void debounce_init(void)
{	
    // Enable pullup on buttons
    B1_PORT |= (BUTTON_MASK);
	PORTE.PIN2CTRL = PORT_PULLUPEN_bm;
}
