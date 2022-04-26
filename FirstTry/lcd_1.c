/*
LCD16x2 4 bit ATmega4809 interface .kbv
http://www.electronicwings.com
*/



//#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#define F_CPU 20000000			/* .kbv Define CPU Frequency e.g. here 2.67MHz */
//.kbv edit defines to suit your wiring.  Note you MUST connect RW pin to GND
//  RS  RW  EN  D4  D5  D6  D7
// PD1 GND PD0 PD4 PD5 PD6 PD7

#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */
#include "lcd.h"

#define D4_PORT PORTC.OUT
#define D4_DIR PORTC.DIR
#define D4_PIN 6
#define D5_PORT PORTB.OUT
#define D5_DIR PORTB.DIR
#define D5_PIN 2
#define D6_PORT PORTF.OUT
#define D6_DIR PORTF.DIR
#define D6_PIN 4
#define D7_PORT PORTA.OUT
#define D7_DIR PORTA.DIR
#define D7_PIN 1
#define RS_PORT PORTA.OUT
#define RS_DIR PORTA.DIR
#define EN_PORT PORTF.OUT
#define EN_DIR PORTF.DIR
#define LCD_Dir  PORTD.DIR			/* .kbv Define LCD data port direction */
#define LCD_Port PORTD.OUT			/* .kbv Define LCD data port */
#define RS 0				/* .kbv Define Register Select pin */
#define EN 5 				/* .kbv Define Enable signal pin */

void DATA_OUT(unsigned char CMND)
{
	D4_PORT &= ~(1<<D4_PIN);
	D5_PORT &= ~(1<<D5_PIN);
	D6_PORT &= ~(1<<D6_PIN);
	D7_PORT &= ~(1<<D7_PIN);
	if(CMND & (1<<4))
	{
		D4_PORT |= (1<<D4_PIN);
	}
	if(CMND & (1<<5))
	{
		D5_PORT |= (1<<D5_PIN);
	}
	if(CMND & (1<<6))
	{
		D6_PORT |= (1<<D6_PIN);
	}
	if(CMND & (1<<7))
	{
		D7_PORT |= (1<<D7_PIN);
	}
}

void LCD_Command( unsigned char cmnd )
{ 
	DATA_OUT(cmnd & 0xF0);	/* sending upper nibble */
	RS_PORT &= ~ (1<<RS);		/* RS=0, command reg. */
	EN_PORT |= (1<<EN);		/* Enable pulse */
	_delay_us(1);
	EN_PORT &= ~ (1<<EN);

	_delay_us(200);

	DATA_OUT(cmnd << 4); 	/* sending upper nibble */
	EN_PORT |= (1<<EN);
	_delay_us(1);
	EN_PORT &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned char data )
{
	DATA_OUT(data & 0xF0); /* sending upper nibble */
	RS_PORT |= (1<<RS);		/* RS=1, data reg. */
	EN_PORT|= (1<<EN);
	_delay_us(1);
	EN_PORT &= ~ (1<<EN);

	_delay_us(200);

	DATA_OUT(data << 4); /* sending lower nibble */
	EN_PORT |= (1<<EN);
	_delay_us(1);
	EN_PORT &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	
	RS_DIR |= (1<<RS);
	EN_DIR |= (1 << EN);			/* Make LCD port direction as o/p */
	D4_DIR |= (1 << D4_PIN);
	D5_DIR |= (1 << D5_PIN);
	D6_DIR |= (1 << D6_PIN);
	D7_DIR |= (1 << D7_PIN);
	RS_PORT &= ~(1<<RS);
	EN_PORT &= ~(1<<EN);
	D4_PORT &= ~(1 << D4_PIN);
	D5_PORT &= ~(1 << D5_PIN);
	D6_PORT &= ~(1 << D6_PIN);
	D7_PORT &= ~(1 << D7_PIN);
	_delay_ms(30);			/* .kbv 30ms is safer with Curiosity voltage control */
	LCD_Command(0x33);		/* .kbv sets regular 8-bit mode */
	_delay_ms(5);           /* .kbv needs delay */
	LCD_Command(0x32);		/* .kbv 4 bit initialization of LCD  */
	LCD_Command(0x28);              /* 2 line, 5*7 matrix in 4-bit mode */
	LCD_Command(0x0c);              /* Display on cursor off*/
	LCD_Command(0x06);              /* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              /* Clear display screen*/
	_delay_ms(2);
}


void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
}