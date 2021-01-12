//// includes, variables.....
/*
Proyecto: 
	PROYECTO FINAL
Fichero: 
	mainc
****************************************************************
Descripcion:

	Este es el programa principal encargado de configurar e inicializar respectivos perifericos que vá a usar.

	Cada periferico tiene un típo de configuración la cual vá a estar definida 
	en su respectiva librería.

*/
// LIBRERIAS
#include <p24HJ256GP610A.h>
#include "explorer16.h"
#include "oscilador.h"
#include "IO.h"
#include "timer.h"
#include "LCD.h"
#include "memoria.h"
#include "UART.h"
#include "CAD.h"
#include "PWM.h"
#include "conversiones.h"
/////////////////////////////


int p ;
unsigned int CPU = 0, contador_CPU=0;

int main ()
{   
	// --- Inicializaciones BASICAS
	Inic_Oscilador();
	Inic_Leds (); 
	Inic_Pulsadores ();
	Inic_LCD();
	Inic_UART ();
	Inic_CAD ();
	    
	///////////////////////////////////////////////////////////

	// Visualizar LCD
	line_1();
	puts_lcd(Ventana_START[0],18); 
	line_2(); 
	puts_lcd(Ventana_START[1],18);
	 
	// --- Espera a pulsador S3
	while (PORTDbits.RD6);

	// --- CONFIG PULSADORES Change Notification
	Configurar_CN();
	  
	// --- Inicialziar crono y timers
	Inicializar_crono();
	Inicializar_T1 ();
	Inicializar_T2_LCD ();
	Inicializar_T3 ();
	Inicializar_T5 ();  
	Inicializar_T4 ();
	Inicializar_T6 ();


	// Inicializar DMA0
	UART_DMA();
	// Inicializar CAD-DMA5
	CAD_DMA();
	PrepararPWM ();
	U2TXREG=0; // enviar null por la UART
 	
	// BUBLE PRINCIPAL

	  while (1) 
	  {
	     TMR5 = 0;
	     contador_CPU = 0;
	     
	    Cronometro();
	    ActualizarPWM ();
	    
	    if(CAD_DMA_flag) {Recoger_valor_CAD_int();}
	     
	    
	    for(p=0; p < 10000 * scroll; p++){
		Nop();
	    } 
	    
	    while(!IFS1bits.T5IF){contador_CPU++;}
	    IFS1bits.T5IF = 0;
	    
	    CPU = 100 - ((contador_CPU/88889)*100);
	    conversion_tiempo(&Ventana_LCD[8][13],&Ventana_LCD[8][14],CPU);
	    conversion_tiempo(&Ventana_LCD_DMA[8][16],&Ventana_LCD_DMA[8][17],CPU);
	    
	  }<// Fin bucle

	  return(0);
} // Fin MAIN

