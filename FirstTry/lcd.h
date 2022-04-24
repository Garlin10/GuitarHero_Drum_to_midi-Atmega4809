
#include <avr/io.h>
#include <util/delay.h>

#ifndef LCD_SHIT
#define LCD_SHIT

#define lcdPort PORTD_OUT                   //Data Output value of PORTC
#define lcdDir PORTD.DIR                    //Setting pins as I/O
//#define EN (PORTD.IN & (1 << 0))            //ENABLE (EN) pin to PORTC PIN0
//#define RS (PORTD.IN & (1 << 1))            //REGISTER SELECT (RS) pin to PORTC PIN1


void LCD_Command( unsigned char cmnd );
void LCD_Char( unsigned char data );
void LCD_Init (void);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);
void LCD_Clear();

#endif