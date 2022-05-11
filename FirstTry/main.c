/*
 * FirstTry.c
 *
 * Created: 2/17/2022 6:16:45 PM
 * Author : User
 */ 

#define F_CPU 20000000
#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "lcd.h" 
#include "debouncer.h"
#include <stdio.h>
#include <string.h>

//EEPROM_READ EEPROM_WRITE
//DEBUGSTUFF
volatile uint16_t sampleTomb[200];
volatile uint16_t n = 0;
//Buffer initialization
#define SIZE_OF_BUFFER 21
volatile uint8_t bufferLength = 0;
volatile uint8_t readIndex = 0;
volatile uint8_t writeIndex = 0;
volatile uint8_t menu_state = 0;
volatile uint8_t buffer[SIZE_OF_BUFFER];
volatile uint8_t channel_looker = 0;
//volatile uint16_t counter[6] = 0;
volatile uint8_t note_velocity[6];
const uint16_t min_velocity[6] = {10,10,20,32,20,300};
volatile uint16_t hit_couter[6] = {0};
volatile uint16_t time_note = 120;
volatile uint16_t actual_max_velo[6];
const uint8_t note_on = 0b10010000;
const uint8_t note_C[6] = {0,1,2,3,4, 5};
void sending(uint8_t note_switch,uint8_t note_NOTE,uint8_t note_volume);
void put_to_buffer(uint8_t note_switch,uint8_t note_NOTE,uint8_t note_volume);
volatile Button_Machine Button_Machines[4] = {Released,Released,Released,Released};
volatile uint8_t BUTTON_FLAGS[4] = {0,0,0,0};
volatile int Button_Timers[4] = {0,0,0,0};
	void LCD_menu(uint8_t state)
	{
		char buf[ 1024];
		switch (state)
		{
			case 0:
			LCD_Command(0x01);
			LCD_String_xy (0, 2, "Guitar Hero");
			LCD_String_xy (1, 3, "Let's Rock");

			_delay_ms(100);
			break;
			case 1:
			LCD_Command(0x01);
			LCD_String_xy (0, 4, "Kick Note");
			sprintf( buf, "%d", note_C[0]);
			LCD_String_xy (1, 8, buf);

			_delay_ms(100);
			break;
			case 2:
			LCD_Command(0x01);
			LCD_String_xy (0, 0, "Kick Velocity");
			
			sprintf( buf, "%d", note_velocity[0]);
			LCD_String_xy (1, 8, buf);

			_delay_ms(100);
			break;
			
		}
	}
void release_state(uint8_t button)
{
	Button_Machines[button] = PushedDown_Wait;
	Button_Timers[button] = 200;
}
void pusheddown_wait_state(uint8_t mask,char port, uint8_t button)
{
	if (Button_Timers[0] < 1)
			{
			if (button_down(mask, port))
				{
					Button_Machines[button] = PushedDown_DoingSomething;
					BUTTON_FLAGS[button] = 1;
					
				}
				else
				{
					Button_Machines[button] = Released;
					
				}
					
			}
}
void pusheddown_doingsomething_state(uint8_t mask,char port, uint8_t button,char* State_Name )
{
	if (BUTTON_FLAGS[button] == 1)
			{
				if(button == 0 || button == 1)
				{
					LCD_menu(menu_state);
					menu_state = (menu_state+1)%(3);
				}
							}
			BUTTON_FLAGS[button] = 0;
			if (!(button_down(mask, port)))
			{
				Button_Timers[button] = 200;
				Button_Machines[button] = Released_Wait;
			}
}
void released_wait_state(uint8_t mask,char port, uint8_t button)
{
	if (Button_Timers[button] < 1)
			{
				if (button_down(mask, port))
				{
					Button_Machines[button] = PushedDown_DoingSomething;
				}
				else
				{
					Button_Machines[button] = Released;
				}
			}
}

	/*0 Left
	  1 Right
	  2 Up
	  3 Down
	 */

enum states{Default_state, There_was_hit};
enum states state[6] = {Default_state,Default_state,Default_state,Default_state,Default_state,Default_state};
int main(void)
{
	// Init clock source.
	ccp_write_io((uint8_t*)&CLKCTRL.MCLKCTRLB, 0x00);
	
	// Set PORTMUX.
	PORTMUX.USARTROUTEA |= PORTMUX_USART3_ALT1_gc;
	
	// Set port directions.
	PORTB.DIR = PIN4_bm;
	//PORTD.DIR = PIN6_bm;
	/* Replace with your application code */
	PORTD.OUT = 0x00;
	
	// Init UART.
	USART3.BAUD = 313;
	USART3.CTRLB = USART_TXEN_bm;
	
	// Init TCB0 timer.
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CNT = 0;
	TCB0.CCMP = 330;
	TCB0.CTRLA = TCB_ENABLE_bm;
	
	//Print HELLO LCD
	LCD_Init();
	LCD_String("hello");
	LCD_Command(0x01);              /* Clear display screen*/
	LCD_String_xy (0, 5, "hello");
	LCD_String_xy (1, 5, "world");
	LCD_Command(0x01); 
	//LCD_Command(0x01);              /* Clear display screen*/
	//BUTTON INIC
	debounce_init();
	//ADC
	//VDD 1 << 4
	PORTD.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTD.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	ADC0.CTRLC = ADC_SAMPCAP_bm | ADC_REFSEL0_bm | ADC_PRESC_DIV16_gc;
	ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;
	ADC0.INTCTRL = ADC_RESRDY_bm;
	ADC0.CTRLA = ADC_ENABLE_bm;

	//ADC0.CTRLC = ADC_REFSEL0_bm; 
	// Enable global interrupts
    sei();
	while (1)
	{
		//B1_FLAG-et csinálni, hogy le van-e nyomva
		//IDe hívom a gombot
		//ELsõ állapt az TIMER állítása
		//GOMB lenyomva/Vissza
		//Utolsó állapot a TIMER ÁLLÍtása
		//GOmb felengedve/vissza
		switch(Button_Machines[0])
		{			
			case Released:
			if (button_down(BUTTON1_MASK, 'E'))
			{      
				release_state(0);
			}
			break;
			case PushedDown_Wait:
			pusheddown_wait_state(BUTTON1_MASK,'E',0);
			break;
			case PushedDown_DoingSomething:
			pusheddown_doingsomething_state(BUTTON1_MASK, 'E', 0,"Left" );
			break;
			case Released_Wait:
			released_wait_state(BUTTON1_MASK,'E', 0);
			default:
			Button_Machines[0] = Released;
			break;
				
		}
	
		switch(Button_Machines[1])
		{			
			case Released:
			if (button_down(BUTTON2_MASK, 'E'))
			{      
				release_state(1);
			}
			break;
			case PushedDown_Wait:
			pusheddown_wait_state(BUTTON2_MASK,'E',1);
			break;
			case PushedDown_DoingSomething:
			pusheddown_doingsomething_state(BUTTON2_MASK, 'E', 1,"Rihtt" );
			break;
			case Released_Wait:
			released_wait_state(BUTTON2_MASK,'E', 1);
			default:
			Button_Machines[1] = Released;
			break;
				
		}
		switch(Button_Machines[2])
		{			
			case Released:
			if (button_down(BUTTON3_MASK, 'E'))
			{      
				release_state(2);
			}
			break;
			case PushedDown_Wait:
			pusheddown_wait_state(BUTTON3_MASK,'E',2);
			break;
			case PushedDown_DoingSomething:
			pusheddown_doingsomething_state(BUTTON3_MASK, 'E', 2,"Down" );
			break;
			case Released_Wait:
			released_wait_state(BUTTON2_MASK,'E', 2);
			default:
			Button_Machines[2] = Released;
			break;
				
		}
		switch(Button_Machines[3])
		{			
			case Released:
			if (button_down(BUTTON4_MASK, 'B'))
			{      
				release_state(3);
			}
			break;
			case PushedDown_Wait:
			pusheddown_wait_state(BUTTON4_MASK, 'B',3);
			break;
			case PushedDown_DoingSomething:
			pusheddown_doingsomething_state(BUTTON4_MASK, 'B',3,"Up" );
			break;
			case Released_Wait:
			released_wait_state(BUTTON4_MASK, 'B',3);
			default:
			Button_Machines[3] = Released;
			break;
				
		}
		
	}
	
}

ISR(TCB0_INT_vect){
	TCB0.INTFLAGS = TCB_CAPT_bm;
	if (Button_Timers[0]>0)
	{
		Button_Timers[0] --;
	}
	if (Button_Timers[1]>0)
	{
		Button_Timers[1] --;
	}
	if (Button_Timers[2]>0)
	{
		Button_Timers[2] --;
	}
	if (Button_Timers[3]>0)
	{
		Button_Timers[3] --;
	}
	switch(channel_looker)
	{
		case 0:
		ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;
		
		break;
		
		case 1:
		ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;
		break;
		
		case 2:
		ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;
		break;
		
		case 3:
		ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;
		break;
		
		case 4:
		ADC0.MUXPOS = ADC_MUXPOS_AIN4_gc;
		break;
		
		case 5:
		ADC0.MUXPOS = ADC_MUXPOS_AIN5_gc;
		break;
		
		//Ide egy iemr változó mindegyik gombra És ezt a Timert csökkentem de 0 alá ne menjen
	}
	ADC0.COMMAND = ADC_STCONV_bm;
}
ISR(ADC0_RESRDY_vect)
{
	
	uint16_t sample = ADC0.RES;
	/*if (n<200){
	sampleTomb[n] = sample;
	n++;
	}
	else
	{
		n = 0;
	}*/
	switch(state[channel_looker])
	{
		case Default_state:
		if (sample > min_velocity[channel_looker]){
			actual_max_velo[channel_looker] = sample;
			state[channel_looker] = There_was_hit;
		}
		else
		{
			channel_looker = (channel_looker+1)%(5);
		}
		//Növeli a hit timert, ha már volt ütés
		if (hit_couter[channel_looker] < time_note && hit_couter[channel_looker] > 0)
		{
			hit_couter[channel_looker] ++;
		}
		//belelír, ha volt ütés és lejárt az idõ
		if(hit_couter[channel_looker] > time_note)
		{
			note_velocity[channel_looker] = (uint8_t)actual_max_velo[channel_looker];
			if(note_velocity[channel_looker] > 127)
			{
				note_velocity[channel_looker] = 127;
			}
				put_to_buffer(note_on,note_C[channel_looker],note_velocity[channel_looker]);
			
			
			//GND-t mérni váltások között TÖRÖLN
			/*ADC0.MUXPOS = ADC_MUXPOS_GND_gc;
			sample = ADC0.RES;*/
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			hit_couter[channel_looker] = 0;
			//Channel change
			channel_looker = (channel_looker+1)%(5);
		}
		break;
		case There_was_hit:
		if (actual_max_velo[channel_looker] < sample)
		{
			actual_max_velo[channel_looker] = sample;
		}
		if (hit_couter[channel_looker] < time_note)
		{
			hit_couter[channel_looker] ++;
		}
		else
		{
			
			note_velocity[channel_looker] = (uint8_t)actual_max_velo[channel_looker];
			if(note_velocity[channel_looker] > 127)
			{
				note_velocity[channel_looker] = 127;
			}
				put_to_buffer(note_on,note_C[channel_looker],note_velocity[channel_looker]);
			
			
			//GND-t mérni váltások között TÖRÖLN
			/*ADC0.MUXPOS = ADC_MUXPOS_GND_gc;
			sample = ADC0.RES;*/
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			hit_couter[channel_looker] = 0;
			state[channel_looker] = Default_state;
			channel_looker = (channel_looker+1)%(5);
		}
	}
	
	
	
	
}

/*void sending(uint8_t note_switch,uint8_t note_NOTE,uint8_t note_volume)
{
	while(!(USART3.STATUS & USART_DREIF_bm))
	{
		
	}
	USART3.TXDATAL = note_switch;
	while(!(USART3.STATUS & USART_DREIF_bm))
	{
		
	}
	USART3.TXDATAL = note_NOTE;
	while(!(USART3.STATUS & USART_DREIF_bm))
	{
		
	}
	USART3.TXDATAL = note_volume;
}*/

void put_to_buffer(uint8_t note_switch,uint8_t note_NOTE,uint8_t note_volume)
{
	if(bufferLength + 3 > SIZE_OF_BUFFER)
	 {
		 return;
	 }
	 
	 buffer[writeIndex] = note_switch;
	 writeIndex = (writeIndex + 1)%(SIZE_OF_BUFFER);
	 buffer[writeIndex] = note_NOTE;
	 writeIndex = (writeIndex + 1)%(SIZE_OF_BUFFER);
	 buffer[writeIndex] = note_volume;
	 writeIndex = (writeIndex + 1)%(SIZE_OF_BUFFER);
	 bufferLength += 3;
	 USART3.CTRLA |= USART_DREIE_bm; 
}
 
ISR(USART3_DRE_vect)
{
	
	if(bufferLength == 0)
	{
		USART3.CTRLA &= ~USART_DREIE_bm;
		
	}
	else
	{
		USART3.TXDATAL = buffer[readIndex];
		readIndex = (readIndex + 1)%(SIZE_OF_BUFFER);
		bufferLength --;
	}
}