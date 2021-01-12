/*
Fichero: LCD.h

Cabeceras de las funciones del modulo LCD.c
*/

void Inic_LCD();	
void lcd_cmd(char cmd);	
void lcd_data(char data);
void puts_lcd (unsigned char *data, unsigned char count);
void line_1();
void line_2();

