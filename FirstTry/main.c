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

//DEBUGSTUFF
volatile uint16_t sampleTomb[200];
volatile uint16_t n = 0;
//Buffer initialization
#define SIZE_OF_BUFFER 21
volatile uint8_t bufferLength = 0;
volatile uint8_t readIndex = 0;
volatile uint8_t writeIndex = 0;
volatile uint8_t buffer[SIZE_OF_BUFFER];
volatile uint8_t channel_looker = 0;
//volatile uint16_t counter[6] = 0;
volatile uint8_t note_velocity[6];
const uint16_t min_velocity[6] = {10,10,20,32,20,20};
volatile uint16_t hit_couter[6] = {0};
volatile uint16_t time_note = 120;
volatile uint16_t actual_max_velo[6];
const uint8_t note_on = 0b10010000;
const uint8_t note_C[6] = {25,36,0b00000110,0b00000111,15, 0b00001101};
void sending(uint8_t note_switch,uint8_t note_NOTE,uint8_t note_volume);
void put_to_buffer(uint8_t note_switch,uint8_t note_NOTE,uint8_t note_volume);

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
	PORTD.DIR = PIN6_bm;
	/* Replace with your application code */
	PORTD.OUT = 0x00;
	
	//Print HELLO LCD
	lcdInit();
	lcdString("hello");
	
	// Init UART.
	USART3.BAUD = 313;
	USART3.CTRLB = USART_TXEN_bm;
	
	// Init TCB0 timer.
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CNT = 0;
	TCB0.CCMP = 330;
	TCB0.CTRLA = TCB_ENABLE_bm;
	
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
		asm("NOP");
		//if (TCB0.CNT > 1500) PORTD.OUT = 0x00;
		//if(TCB0.INTFLAGS & 1) PORTD.OUT = 0x00;
		/*if(USART3.STATUS & USART_DREIF_bm)
		{
			USART3.TXDATAL = 'U';
			PORTD.OUT = 0x40;
			_delay_ms(1000);
			PORTD.OUT = 0x00;
		_delay_ms(1000);
		}*/
		
	}
}

ISR(TCB0_INT_vect){
	TCB0.INTFLAGS = TCB_CAPT_bm;
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
		
	}
	ADC0.COMMAND = ADC_STCONV_bm;
	/*uint16_t counter;
	if (counter >=10000)
	{
		
		counter ++;
		//portd.out = 0x40;
		if (counter >= 20000)
		{
			counter = 0;
		
		}
		
		
	}
	else
	{
		counter ++;
		portd.out = 0x00;
		
	}*/
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
			channel_looker = (channel_looker+1)%(6);
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
			channel_looker = (channel_looker+1)%(6);
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
			channel_looker = (channel_looker+1)%(6);
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