/*
Fichero: 
	timer.c

 Descripción
	Funciones para la gestion de timers
 */

// Modulos y Librerias
#include <p24HJ256GP610A.h>
#include "explorer16.h"
#include "conversiones.h"
#include "LCD.h"
#include "memoria.h"
#include "PWM.h"

//Variables globales
unsigned int ms, ds, s, min, scroll = 0,i; 
static int data, estadoLCD;

/*
 - Nombre: Inicializar_T1
 - Descripción
	inicializar T1 para contar 1 ms (por encuesta)
 */
void Inicializar_T1 ()
{
  TMR1=0; 	
  PR1=40000-1;	
  T1CON=0;
  T1CONbits.TCKPS=0;	
  
  // CONFIG INTERRUPTIONS
  IFS0bits.T1IF=0;
  IEC0bits.T1IE=1;
  IPC0bits.T1IP=4;
}

/*
 - Nombre: Inicializar_T3
 - Descripción
	inicializar T1 para contar 1 ms (por encuesta)
 */

void Inicializar_T3 ()
{
  TMR3=0; 	
  PR3=50000;	
  T3CON=0;
  T3CONbits.TCKPS=1;	// Prescaler 8
  
  // CONFIG INTERRUPTIONS
  IFS0bits.T3IF=0;
  IEC0bits.T3IE=0; // habilitar int
  IPC2bits.T3IP=4;
 
  T3CONbits.TON=1;
}

void Inicializar_T2_LCD ()
{
  TMR2=0; 	
  PR2=10000;	
  T2CON=0;
  T2CONbits.TCKPS=1;	// Prescaler 8
  
  // CONFIG INTERRUPTIONS
  IFS0bits.T2IF=0;
  IEC0bits.T2IE=1; // habilitar int
  IPC1bits.T2IP=4;
 
}

void Inicializar_T4 ()
{  
  // CONFIG INTERRUPTIONS
  IFS1bits.T4IF=0; // Flag T4 a 0
  IEC1bits.T4IE=0; // Por encuesta (Desabilitar interrupcion)
  IPC6bits.T4IP=4; // Prioridad 4
}

void Inicializar_T5 ()  // 20ms
{
  TMR5=0; 	
  PR5=12500;	
  T5CON=0;
  T5CONbits.TCKPS=3;	// Prescaler 64
  
  // CONFIG INTERRUPTIONS
  IFS1bits.T5IF=0;
  IEC1bits.T5IE=0; // habilitar int
  IPC7bits.T5IP=4;
  
  T5CONbits.TON=1; 
}

void Inicializar_T6 ()  // 20ms
{
  TMR6=0; 	
  PR6=T_MINPWM;	
  T6CON=0;
  T6CONbits.TCKPS=3;	// Prescaler 64
  
  // CONFIG INTERRUPTIONS
  IFS2bits.T6IF=0;
  IEC2bits.T6IE=1; // habilitar int
  IPC11bits.T6IP=4;
 
}

void _ISR_NO_PSV _T1Interrupt () {
    ms++;
    IFS0bits.T1IF=0;
}

void Inicializar_crono()		
{
	ms=0;	
	ds=0;			
	s=0;			
	min=0;
}


void Delay_T4_us (unsigned int nus)
{
	if (nus>1000) LATAbits.LATA5=1; // demasiado grande (1:1)
    else{
        // completar el codigo para prescaler (1:1)
        
        // Configurar el timer
        TMR4 = 0; 	
        PR4 = nus / 0.025; 	 // nus / 25ns
        T4CON=0;
        T4CONbits.TCKPS=0; // Prescaler : 1
        
        // activar T4
        T4CONbits.TON=1;
        
        //Espera por encuesta
        while(!IFS1bits.T4IF);
        IFS1bits.T4IF=0;
        T4CONbits.TON=0;
     }

}

void Delay_T4_ms (unsigned int nms)
{
  if (nms>100) LATAbits.LATA5=1; // demasiado grande (1:64)
  else
  {
  	// completar el codigo en funcion del prescaler (1:1, 1:8, 1:64)
    if(nms < 1){
        
        // Configurar el timer
        TMR4 = 0; 	
        PR4 = nms / 0.000025; 	
        T4CON=0;
        T4CONbits.TCKPS=0; // Prescaler : 1}
    }
    
    if((1 <= nms) && (nms < 13)){
        
        // Configurar el timer
        TMR4 = 0; 	
        PR4 = nms / 0.0002; 	
        T4CON=0;
        T4CONbits.TCKPS=1; // Prescaler : 8}
    }
    
    if(13 <= nms){
        
        // Configurar el timer
        TMR4 = 0; 	
        PR4 = nms / 0.0016; 	
        T4CON=0;
        T4CONbits.TCKPS=2; // Prescaler : 64}
    }
    
    // activar timer T1
    T4CONbits.TON=1;
        
    //Espera por encuesta
    while(!IFS1bits.T4IF);
    IFS1bits.T4IF=0;
    T4CONbits.TON=0;
  }

}


void _ISR_NO_PSV _T2Interrupt () {
    
    switch(estadoLCD){
    
        case 0:
            lcd_cmd(0x80); //line_1
            estadoLCD = 1;
            data = 0;
            break;
            
        case 1:
       
            lcd_data(Ventana_LCD[scroll][data++]);
            if(data == 18) {
                estadoLCD = 2;
            }
            break;
            
        case 2:
            lcd_cmd(0xC0);  //line_2
            estadoLCD = 3;
            data = 0;
            break;
            
        case 3:
            if(scroll == 9){lcd_data(Ventana_LCD[0][data++]);}
            else{lcd_data(Ventana_LCD[scroll+1][data++]);}
            
            
            if(data == 18) {
                estadoLCD = 0;
            }
            break;
    }
    
    IFS0bits.T2IF=0;
}


//////////////////////////////////////

void Cronometro()
{                 
  if (ms>=100) 
  {  
     ms=0;  
     LATAbits.LATA0=!LATAbits.LATA0;     
     ds++;
     if (ds==10) {
         ds=0; 
         LATAbits.LATA1=!LATAbits.LATA1; 
         s++;}
     if (s==60) {s=0; LATAbits.LATA2=!LATAbits.LATA2; min++;}
     
     // Visualizar en el LCD	
     conversion_tiempo(&Ventana_LCD[1][8],&Ventana_LCD[1][9],min);
     conversion_tiempo(&Ventana_LCD[1][11],&Ventana_LCD[1][12],s);
     conversion_tiempo(&Ventana_LCD[1][14],&Ventana_LCD[1][14],ds);
     
     // Visualizar UART por DMA	
     conversion_tiempo(&Ventana_LCD_DMA[1][10],&Ventana_LCD_DMA[1][11],min);
     conversion_tiempo(&Ventana_LCD_DMA[1][13],&Ventana_LCD_DMA[1][14],s);
     conversion_tiempo(&Ventana_LCD_DMA[1][16],&Ventana_LCD_DMA[1][16],ds);
   }  	
}

