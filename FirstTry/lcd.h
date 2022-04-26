
#include <avr/io.h>
#include <util/delay.h>

#ifndef LCD_SHIT
#define LCD_SHIT

#define lcdPort PORTD_OUT                   //Data Output value of PORTC
#define lcdDir PORTD.DIR                    //Setting pins as I/O

void LCD_Command( unsigned char cmnd );
void LCD_Char( unsigned char data );
void LCD_Init (void);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);
void LCD_Clear();

#endif