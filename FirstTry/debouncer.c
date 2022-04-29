/*
  debounce.c
 */

#include "debouncer.h"

// Bits is set to one if a depounced press is detected.
volatile uint8_t buttons_down;

// Return non-zero if a button matching mask is pressed.
//button_mask-ot kivenni
//20ms várok utána váltok csak állapotot
uint8_t button_down(uint8_t button_mask, char port)
{
	uint8_t PORT_INFO  = PORTE.IN;
	switch(port)
	{
		case 'E':
		PORT_INFO = PORTE.IN;	
		break;
		case 'B':
		PORT_INFO = PORTB.IN;
		break;
	}
    
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
	//Bal
	E2_PORT |= (BUTTON1_MASK);
	E1_PORT |= (BUTTON2_MASK);
	E0_PORT	|= (BUTTON3_MASK);
	
    B1_PORT |= (BUTTON4_MASK);
	
	PORTE.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTE.PIN1CTRL = PORT_PULLUPEN_bm;
	PORTE.PIN0CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN1CTRL = PORT_PULLUPEN_bm;
}
