/*
LCD16x2 4 bit ATmega4809 interface .kbv
http://www.electronicwings.com
*/



//#define F_CPU 8000000UL			/* Define CPU Frequency e.g. here 8MHz */
#define F_CPU 2666667UL			/* .kbv Define CPU Frequency e.g. here 2.67MHz */
//.kbv edit defines to suit your wiring.  Note you MUST connect RW pin to GND
//  RS  RW  EN  D4  D5  D6  D7
// PD1 GND PD0 PD4 PD5 PD6 PD7

#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include Delay header file */
#include "lcd.h"

#define LCD_Dir  PORTD_DIR			/* .kbv Define LCD data port direction */
#define LCD_Port PORTD_OUT			/* .kbv Define LCD data port */
#define RS 2				/* .kbv Define Register Select pin */
#define EN 3 				/* .kbv Define Enable signal pin */


void LCD_Command( unsigned char cmnd )
{
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);		/* RS=0, command reg. */
	LCD_Port |= (1<<EN);		/* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_Port |= (1<<RS);		/* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Dir = 0xFF;			/* Make LCD port direction as o/p */
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

#if 0
int main()
{

	LCD_Init();			/* Initialization of LCD*/

	LCD_String("LCD_PORTD");	/* Write string on 1st line of LCD*/
	//LCD_Command(0xC0);		/* Go to 2nd line*/
	//LCD_String("Hello World");	/* Write string on 2nd line*/
	LCD_String_xy(1, 4, "Hello World");	/* .kbv set cursor to 4, 1 and print string*/
	while(1);
}
#endif
