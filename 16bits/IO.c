/*
Fichero:
	IO.c

Descripcion:
	Configuraciones de las IO y sus respectivas interrupciones asociadas
*/
#include <p24HJ256GP610A.h>
#include "explorer16.h"
#include "timer.h"

/*
Nombre:
	Inic_Leds
Descripcion:
	Inicializa los LEDS
	LEDs (LED3-LED10)
*************************************************************************************************************************************************
*/
void Inic_Leds ()
{
	// LEDs en el puerto A: RA7-RA0 salida
	TRISA = 0xFF00;		

	TRIS_LED_D3 = 0;
	TRIS_LED_D4 = 0;
	TRIS_LED_D5 = 0;
	TRIS_LED_D6 = 0;
	TRIS_LED_D7 = 0;
	TRIS_LED_D8 = 0;

} 


/*
Nombre:
	Inic_Pulsadores ()
Descripcion:
	Inicializa los Pulsadores del Explorer16
	PULSADORES (S3,S4,S5,S6)
*************************************************************************************************************************************************
*/
void Inic_Pulsadores ()
{
	// los pines analogicos estan configurados como analogicos
	// definir todos los pines como digitales
	// mas adelante (CAD) configuraremos los que nos interesen como analogicos
	// asi no hay problemas con el pin RA7 (LED 10 / Pulsador S5)
	    
	AD1PCFGL= 0xFFFF;   // Pone todas las patas anal?gicas de I/O digital
	AD1PCFGH= 0xFFFF;   

	// configurar el comportamiento E/S: TRIS (1)
	Pulsador_S3_TRIS = 1;
	Pulsador_S4_TRIS = 1;
	Pulsador_S5_TRIS = 1;
	Pulsador_S6_TRIS = 1;
} 


void Configurar_CN ()
{
		// configurar pulsadores por interrupcion 
    
    CNEN2bits.CN19IE=1; // Habilita interrupci?n pulsador S4
    CNEN2bits.CN16IE=1; // Habilita interrupci?n pulsador S6
    CNEN1bits.CN15IE=1; // Habilita interrupci?n pulsador S3
    CNEN2bits.CN23IE=1; // Habilita interrupci?n pulsador S5
    
    
    IPC4bits.CNIP = 4; // Nivel de prioridad (de 1 m?nima a 7 m?xima)
    IFS1bits.CNIF = 1; // Borra flag interrupci?n CN
    IEC1bits.CNIE = 1; // Habilita interrupciones CN

}

/*
Nombre:
	_CNInterrupt ()
Descripcion:
 -------------- INTERRUPCION PULSADORES ---------
*************************************************************************************************************************************************
*/
void _ISR_NO_PSV _CNInterrupt () 
{
     // START: Encender Relojes T1 & T2 
    if(!START_S3)
    {
        T1CONbits.TON=1;
        T2CONbits.TON=1; 
    } 
    // STOP: Apagar Relojes T1 
    if(!STOP_S6){
        T1CONbits.TON=0;
    }
    // RESET: Inicializar valores del crono
    if(!RESET_S5){
        Inicializar_crono();
    } 
    // SCROLL: Cambiar linea de visualización del LCD
    if(!SCROLL_S4){
        if(scroll == 9){
            scroll = 0;
        }
        else{
         scroll++;
        }
        
    } 

    
    IFS1bits.CNIF = 0; // Resetear flag de pulsador

}
