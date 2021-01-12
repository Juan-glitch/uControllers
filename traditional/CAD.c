/*
Fichero: 
	CAD.c

Descripcion:
	Funciones basicas del CAD
 */

//MODULOS Y LIBRERIAS
#include <p24HJ256GP610A.h>
#include "explorer16.h"
#include "memoria.h"
#include "conversiones.h"
#include "PWM.h"

// Variables locales
int adcFlag = 0, a = 0, b = 0, CAD_DMA_flag = 0;
int val_pot[8], val_temp[8];
unsigned int valor, canal;
unsigned long contador = 0;

int resultado[4], PP = 0;

void Inic_CAD ()
{
	// Inicializacion registro control AD1CON1
	AD1CON1 = 0;       // todos los campos a 0

	// Salida de 12 bits o 10 bits
	//AD1CON1bits.AD12B = 0;  

	AD1CON1bits.ADDMABM = 1;// 1 = order / 0 = scatter/gather

	// Comienzo digitalizacion automatico
	// 111=Auto-convert / 010=TMR3 ADC1 y TMR5 ADC2 / 001=INT0 / 000= SAMP 
	AD1CON1bits.SSRC=2;    		

	// Muestreo simultaneo o secuencial
	//AD1CON1bits.SIMSAM = 0; 
		     
	// Comienzo muestreo autom√°tico o por programa (SAMP=1) 		
	AD1CON1bits.ASAM = 1;
	     
		     
	// Inicializacion registro control AD1CON2
	AD1CON2= 0 ;  // todos los campos a 0
	AD1CON2bits.CSCNA = 1;
	AD1CON2bits.SMPI = 4 - 1; //Numero canales muestreo


	// Inicializacion registro control AD1CON3
	AD1CON3=0;    // todos los campos a 0
	// Reloj con el que funciona el ADC:  0= reloj CPU; 1= reloj RC 
	//AD1CON3bits.ADRC = 0;  // 
	AD1CON3bits.SAMC=31;   // Tiempo muestreo = numero de Tad 
	AD1CON3bits.ADCS=3;    // Relacion entre TAD y Tcy TAD = Tcy(ADCS+1)


	// Inicializacion registro control AD1CON4
	AD1CON4=0;
	AD1CON4bits.DMABL = 3; // Datos por canal


	// Inicializacion registro AD1CHS123
	AD1CHS123=0;	//seleccion del canal 1,2 eta 3


	// Inicializacion registro AD1CHS0
	AD1CHS0=0;
	//AD1CHS0bits.CH0SA=POT;  // elige la entrada analogica conectada
	// AN5, potenciometro
	//AD1CHS0bits.CH0SB = 0;


	// Inicializacion registros AD1CSS 
	// Si escaneo secuencial 1, si no 0
	AD1CSSH = 0;   // 16-31 
	AD1CSSL = 0b110011;   // 0-15 

	// Inicializacion registros AD1PCFG. Inicialmente todas AN como digitales
	AD1PCFGH = 0xFFFF;      // 1= digital / 0= Analog
	AD1PCFGL = 0xFFFF;      // Puerto B, todos digitales
	// Inicializar como analogicas solo las que vayamos a usar
	AD1PCFGLbits.PCFG5=0;   // potenciometro
	AD1PCFGLbits.PCFG4=0;   // sensor de temperatura
	AD1PCFGLbits.PCFG0=0;   // JOYSTICK X
	AD1PCFGLbits.PCFG1=0;   // JOYSTICK Y


	// Bits y campos relacionados con las interrupciones
	IFS0bits.AD1IF=0;    
	IEC0bits.AD1IE=0;    
	IPC3bits.AD1IP=4;    

	//AD1CON
	AD1CON1bits.ADON=1;  // Habilitar el modulo ADC

	contador = 0;
}


/*
Nombre:
	Media
Descripcion:
	Devuelve la media de 8 conversiones y la guarda en un vector global
	para su posterior uso
*************************************************************************************************************************************************
*/
void media(){
    
    int i,j;
    
    switch(PP){
        
        case 0: 
            for(j=0; j < 4; j++){
                for(i=0; i < 8; i++){
                 resultado[j] = resultado[j] + ((CAD_BuffB[(i * 4) + j]) / 8);
                }
            }
            break;
      
        case 1:
            for(j=0; j < 4; j++){
                for(i=0; i < 8; i++){
                 resultado[j] = resultado[j] + ((CAD_BuffA[(i * 4) + j]) / 8);
                }
            }
            break;
            
    }
}
/*
Nombre:
	Recoger_valor_CAD
Descripcion:
	Recoge la media de 8 conversiones y las deposita en su lectura 
	analogica correspondiente para su visualizaciÛn
*************************************************************************************************************************************************
*/
void Recoger_valor_CAD_int(){


    
    media();
    
    // XJOY canal 0
     Conversion_CAD (&Ventana_LCD[5][13], resultado[0]);
     Conversion_CAD (&Ventana_LCD_DMA[5][16], resultado[0]);
   
    // YJOY canal 1
    Conversion_CAD (&Ventana_LCD[6][13], resultado[1]);
    Conversion_CAD (&Ventana_LCD_DMA[6][16], resultado[1]);
    
    // TEMP canal 2
    Conversion_CAD (&Ventana_LCD[4][13], resultado[2]);
    Conversion_CAD (&Ventana_LCD_DMA[4][16], resultado[2]);
    
    // POT canal 3
    Conversion_CAD (&Ventana_LCD[3][13], resultado[3]);
    Conversion_CAD (&Ventana_LCD_DMA[3][16], resultado[3]);
    
    Cambiar_DUTY(resultado[3]); 
    
  resultado[0] = 0; 
  resultado[1] = 0; 
  resultado[2] = 0; 
  resultado[3] = 0; 
  
  CAD_DMA_flag = 0;
}


/*
Nombre:
	CAD_DMA()
Descripcion:
	Configura el DMA5 para su uso sincronizado con el CAD
*************************************************************************************************************************************************
*/
void CAD_DMA()
{	
    
    DMA5CON=0;
    
    DMA5CONbits.SIZE=0; //Word
    DMA5CONbits.DIR=0; //De periferico a memoria
    DMA5CONbits.HALF=0; // No interrumpir en el medio
    DMA5CONbits.AMODE=0; //Order
    DMA5CONbits.MODE=2; //Continuo + Ping pong
    
    DMA5REQbits.IRQSEL = 0x0D; // CAD
    DMA5STA = __builtin_dmaoffset(CAD_BuffA);
    DMA5STB = __builtin_dmaoffset(CAD_BuffB);
    
    DMA5PAD = (volatile unsigned int)&ADC1BUF0;
    DMA5CNT = 32 - 1; // 8x4 - 1
    
    DMACS0 = 0; // Borra flag de colisiones
    INTCON1bits.DMACERR = 0;  // Borra flag de interrupciÛn de colisiones
    
    IPC15bits.DMA5IP=4;
    IFS3bits.DMA5IF=0;
    IEC3bits.DMA5IE=1; 
    
    DMA5CONbits.CHEN=1;
} 
/*
Nombre:
	_DMA5Interrupt()()
Descripcion:
	InterrupciÛn para habilitar el uso de Ping Pong entre las distintas ubicaciones 
	de lectura y escritura del DMA
*************************************************************************************************************************************************
*/

void _ISR_NO_PSV _DMA5Interrupt()
{
    
    switch(PP){
        
        case 0:  // Buffer A
            PP = 1;
            CAD_DMA_flag = 1;
            break;
            
        case 1:  // Buffer B
            PP = 0;
            CAD_DMA_flag = 1;
            break;
    
    }
    
    IFS3bits.DMA5IF=0;
} 
