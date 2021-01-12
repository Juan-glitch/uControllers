/*
Fichero: 
	LCD.c

Descripcion:
	Funciones básicas de gestion del LCD

Notas:

   For Explorer 16 board, here are the data and control signal definitions
   RS -> RB15
   E  -> RD4
   RW -> RD5
   DATA -> RE0 - RE7   

 */

//MODULOS Y LIBRERIAS
#include <p24HJ256GP610A.h>
#include "timer.h"

	// NOTAS: Control signal data pins 
	#define  RW  LATDbits.LATD5       // LCD R/W signal
	#define  RS  LATBbits.LATB15      // LCD RS signal
	#define  E   LATDbits.LATD4       // LCD E signal 

// Control signal pin direction 
#define  RW_TRIS	TRISDbits.TRISD5 
#define  RS_TRIS	TRISBbits.TRISB15
#define  E_TRIS		TRISDbits.TRISD4

// Data signals and pin direction
#define  DATA      LATE           // Port for LCD data
#define  DATAPORT  PORTE
#define  TRISDATA  TRISE          // I/O setup for data Port






/*
Nombre: 
	lcd_cmd

Descripcion:
	Ejecuta la subrutina correspondiente del modelo LCD
	Para lectura y escritura.
***********************************************************************************************************************************************
 */


void lcd_cmd (char cmd) 
{
	RW = 0;		// RW=0, escritura
	RS = 0;		// RS=0, comando
	DATA &= 0xFF00; // prepare RD0 - RD7
	DATA |= cmd;	// command byte to lcd
	E = 1;		// toggle E line
	Nop();		// 10 NOP, asegurar la temporizacin de E
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	E = 0;		// desactivar E
	Nop();
	RW = 1;		// desactivar escritura
}

void lcd_data (char data)      // subroutine for lcd data
{
  RW = 0;       				// RW=0, escritura
  RS = 1;               // RS=1, dato
  DATA &= 0xFF00;       // prepare RD0 - RD7
  DATA |= data;         // data byte to lcd
  E = 1;                // toggle E line
  Nop();                // 10 NOP, asegurar la temporizacin de 
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  E = 0;            // desactivar E
  Nop();
  RS = 0;           // desactivar RS
  RW = 1;						// desactivar escritura
}



/*
Nombre: 
	Inic_LCD

Descripcion:
	Inicializamos el LCD Display
***********************************************************************************************************************************************
 */

void Inic_LCD ()             // initialize LCD display
{
  // 15mS delay after Vdd reaches nnVdc before proceeding with LCD initialization
  // not always required and is based on system Vdd rise rate
  Delay_T4_ms (15); // 15 ms delay
			
  /* set initial states for the data and control pins */
  DATA &= 0xFF00;	
  RW = 0;                       // R/W state set low
  RS = 0;                       // RS state set low
  E = 0;                        // E state set low

  /* set data and control pins to outputs */
  TRISDATA &= 0xFF00;
  RW_TRIS = 0;                  // RW pin set as output
  RS_TRIS = 0;                  // RS pin set as output
  E_TRIS = 0;                   // E pin set as output

  /* LCD initialization sequence */ 
  lcd_cmd (0x38);				// function set (3 aldiz)
  Delay_T4_ms (5);                 // 5 ms delay
  lcd_cmd (0x38);
  Delay_T4_us (100);               // 200 us delay
  lcd_cmd (0x38);
  Delay_T4_us (40);                // 40 us delay
  lcd_cmd (0x38);  
  Delay_T4_us (40);                // 40 us delay
  lcd_cmd (0x0C);              	// Display on/off control, cursor blink off (0x0C)
  Delay_T4_us (40);                // 40 us delay
  lcd_cmd (0x06);			  				// Entry mode set (0x06)
  Delay_T4_us (40);                // 40 us delay
}


/*
Nombre: 
	puts_lcd

Descripcion:
	Le metemos la posición de la matriz en la que tiene que escribir y empieza su rutina hasta llegar al ultimo valor
	// data + X(cuantos caracteres)
***********************************************************************************************************************************************
 */
void puts_lcd (unsigned char *data, unsigned char count) 
{
  while (count)
	{
	 lcd_data(*data++);
     Delay_T4_us (40);
	 count--;
	}	
}


//Para escribir en la 1 linea del LCD
void line_1() 
{
    lcd_cmd(0x80);  // Set DDRAM address (@0)
    Delay_T4_us (40);  // 40 us delay
}
//Para escribir en la 2 linea del LCD
void line_2() 
{
    lcd_cmd(0xC0);  // Set DDRAM address (@40)
    Delay_T4_us (40);  // 40 us delay
}
