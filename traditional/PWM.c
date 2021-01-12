/*
Fichero: 
	PWM.c

 Descripcion
	Funciones basicas PWM
 */

// MODULOS Y LIBRERIAS

#include <p24HJ256GP610A.h>

#include "explorer16.h"
#include "memoria.h"
#include "conversiones.h"
#include "CAD.h"

// DEFINICIONES
//=========================================================

#define MINPWM 0.4    //Duracion minima pulso PWM (en milisegundos)
#define MAXPWM 2.2    //Duracion maxima pulso PWM (en milisegundos)
#define T20ms 12500   //Periodo adecuado para conseguir 20 ms
                      //Prescaler 1:64. Fcy: 40000000
                      // (40000/64)*20=625*20=12500

//VARIABLES
//=========================================================
unsigned int T_MINPWM=(MINPWM)*(T20ms/20); // valor contador para que el pulso dure MINPWM  250
unsigned int T_MAXPWM=(MAXPWM)*(T20ms/20); // valor contador para que el pulso dure MAXPWM  1375

unsigned int estadoPWM,DUTY,refrescarPWM;



/*
Fichero: 
	PrepararPWM
 Descripcion
	Configura el pin correspondiente a la salida del pulso
**************************************************************************************
 */
void PrepararPWM ()
{
    // inicializar pin D15 para pulso PWM
    TRISDbits.TRISD15=0; //salida
    estadoPWM=PULSO_1;
    DUTY=T_MINPWM;
    T6CONbits.TON=1;
    LATDbits.LATD15=1;
    refrescarPWM=1;
}

/*
Fichero: 
	ActualizarPWM
 Descripcion
	Actualiza los valores de LCD y DMA respecto al estado PWM
**************************************************************************************
 */
void ActualizarPWM () 
{
      if (refrescarPWM==1) 
      {
          Conversion_PWM(&Ventana_LCD[7][12],DUTY);
          Conversion_PWM(&Ventana_LCD_DMA[7][15],DUTY);
          refrescarPWM=0;
      }
}

/*
Fichero: 
	Cambiar_DUTY
 Descripcion
	Cambia el tiempo del pulso en funcion de un valor int

**************************************************************************************
 */

void Cambiar_DUTY(unsigned int valpot) 
 {
     float valor;
    
     valor=T_MINPWM+(valpot/1023.0)*(T_MAXPWM-T_MINPWM);
     DUTY=valor;
     refrescarPWM=1;
 }

/*
Fichero: 
	_T6Interrupt (PWM)
 Descripcion
	Interrupción asociada al timer correspondiente de variar el PWM

**************************************************************************************
 */
void _ISR_NO_PSV _T6Interrupt () {

    switch(estadoPWM){
        
        case PULSO_1:
            estadoPWM=PULSO_0;
            PR6 = T20ms - PR6 -1;
            LATDbits.LATD15=0;
            LED_D7 = 0;
            break;
            
        case PULSO_0:
            estadoPWM=PULSO_1;
            PR6 = DUTY;
            LATDbits.LATD15=1;
            LED_D7 = 1;
            break;
    }
    
      IFS2bits.T6IF=0;
}
