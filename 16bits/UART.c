//////////// vuestro c�digo
/*
Fichero: UART.c

 Descripci�n
 * Funciones b�sicas de la UART
 */

#include <p24HJ256GP610A.h>

#include "explorer16.h"
#include "timer.h"
#include "memoria.h"

#define Fosc 80000000       // Frecuencia de reloj de la CPU (oscillator)
#define Fcy	 Fosc/2		// Velocidad de ejecucion de las instrucciones
#define BAUDRATE2 9600  // Velocidad de transmision de UART2 (baudios)
#define T_1BIT_US (1000000/BAUDRATE2)+1 // Duracion de 1 bit 

#define BAUD_RATEREG_2_BRGH1 ((Fcy/BAUDRATE2)/4)-1	// valor U2BRG si BRGH=1
#define BAUD_RATEREG_2_BRGH0 ((Fcy/BAUDRATE2)/16)-1	// valor U2BRG si BRGH=0

// Definiciones relacionadas con la pantalla del terminal del PC
#define clrscr "\x1b[2J" //4 character: \x1b, [, 2, J
#define home "\x1b[H"    //3 character: \x1b, [, H
#define cursor_off "\x1b?25l"    // cursor no visible

#define CR 0x0D		// carriage return
#define LF 0x0A		// line feed

static int estadoUART = 0;
int data = 0, cambio = 0, indice = 0;


void Inic_UART ()
{
	
  // Velocidad de transmision
  // Hay que considerar una de las dos asignaciones siguientes
	U2BRG = BAUD_RATEREG_2_BRGH1;
	//U2BRG = BAUD_RATEREG_2_BRGH0;

  // U2MODE: habilitar el modulo (UARTEN), 8 bits, sin paridad (PDSEL),
  // 1 bit de stop (STSEL), BRGH ...
	U2MODE=0; // Inicializar todo los bits del registro U2MODE a 0
	U2MODEbits.BRGH=1;

  // U2STA: modo de interrupcion en el envio (UTXISEL), habilitacion del
  // envio (UTXEN), modo de interrupcion en la recepcion (URXISEL)
	U2STA=0; // Inicializar todo los bits del registro U2STA a 0

  // inicializacion de los bits relacionados con las interrupciones RX y TX
	IEC1bits.U2RXIE=1; // Por encuesta
    IFS1bits.U2RXIF=0; // Inicializar falg a 0
    IPC7bits.U2RXIP=4; // Prioridad 4

    IEC1bits.U2TXIE=1;
    IFS1bits.U2TXIF=0;
	IPC7bits.U2TXIP=4;

  // interrupciones debidas a errores + bug
	IEC4bits.U2EIE=1;
	U2STAbits.OERR=0;

  // Habilitar el modulo y la linea TX.
  // Siempre al final y en ese orden!!!
	U2MODEbits.UARTEN=1;    // habilitar UART2
	U2STAbits.UTXEN=1;      // habilitar transmision tras habilitar modulo

	Delay_T4_us(T_1BIT_US); 	// Esperar tiempo de 1 bit 
}

// Envio de un dato mediante UART2 por ENCUESTA
void Put_UART(char c)
{
	// A COMPLETAR!!!!!!
    while(!IFS1bits.U2TXIF)
    U2TXREG = c;
    IFS1bits.U2TXIF = 0;
  

}


void Home_UART ()
{
	Put_UART (home[0]);
	Put_UART (home[1]);
	Put_UART (home[2]);
}

// Envio de la informacion de Ventana_LCD
void Enviar_Ventana_UART ()
{
    int i;
    
	Home_UART();
	// A COMPLETAR!!!!!!
    for(i = 0; i < 16 ; i++){
    
        Put_UART(Ventana_LCD[0][i]);
    }
    
    Put_UART(LF);
    Put_UART(CR);
    
    
      for(i = 0; i < 16 ; i++){
    
        Put_UART(Ventana_LCD[1][i]);
    }
    
    Put_UART(LF);
    Put_UART(CR);
     
}

void _ISR_NO_PSV _U2TXInterrupt ()
{
    switch(estadoUART){
        
        
        case 3:
            //Clear_UART();
             U2TXREG = (clrscr[data]);
            data++;
            if(data == 4){
                estadoUART = 0;
                data = 0;
                indice = 0;
            }
         
            break;
        
        case 0:
            //Home_UART();
             U2TXREG = (home[data]);
            data++;
            if(data == 3){
                estadoUART = 1;
                data = 0;
                indice = 0;
            }
         
            break;
            
        case 1:
       
             U2TXREG =(Ventana_LCD[indice][data++]);
            
            if(data == 18) {
                
                indice++;
                data = 0;
                if(indice == 10){
                    estadoUART = 0;
                    data = 0;
                }
            }
            break;
            
    } 
    
	IFS1bits.U2TXIF = 0;
}

void _ISR_NO_PSV _U2RXInterrupt (){
    
    switch(U2RXREG){
            case 'm':
                T1CONbits.TON=1;
                break;
                
            case 'p':
                T1CONbits.TON=0;
                break;
                
            case 'r':
                Inicializar_crono();
                break;
                
            case 's':
                if(scroll == 9){
                 scroll = 0;
                 }
                else{
                scroll++;
                 }
                break;
            
            case 'c':
           if(cambio == 1)
           {
               cambio = 0; 
               IEC1bits.U2TXIE=0;
               DMA0CONbits.CHEN=1;
               LATAbits.LATA5 = cambio;
               break; 
           }
           if(cambio == 0) cambio = 1;
           break;
                
    }  
   
    IFS1bits.U2RXIF=0;

}

//------------------------------------------------
void _ISR_NO_PSV _U2ErrInterrupt ()
{
	Nop();
	Nop();
  while (1);
}
//////////////////////////////////////

//// UART DMA ------------------
void UART_DMA()
{	
     IEC1bits.U2TXIE=0; // parar interrupciones UART TX
    
    DMA0CON=0;
    DMA0CONbits.SIZE=1; //Byte
    DMA0CONbits.DIR=1; //De memoria a periferico
    DMA0CONbits.HALF=0; // No interrumpir en el medio
    DMA0CONbits.AMODE=0; //Postincremento
    DMA0CONbits.MODE=0; //Cpntinua
    DMA0REQbits.IRQSEL = 0x1F; // UART2_TX
    DMA0STA = __builtin_dmaoffset(Ventana_LCD_DMA);
    DMA0PAD = (volatile unsigned int)&U2TXREG;
    DMA0CNT = (10 * 19) - 1; //Ventana_LCD_DMA[2][19]
    
    DMACS0 = 0; // Borra flag de colisiones
    INTCON1bits.DMACERR = 0;  // Borra flag de interrupci?n de colisiones
    
    IPC1bits.DMA0IP=4;
    IFS0bits.DMA0IF=0;
    IEC0bits.DMA0IE=1; 
    
    DMA0CONbits.CHEN=1;
} 


// === Interrupcion DMA ===================
void _ISR_NO_PSV _DMA0Interrupt()
{
   if(cambio == 1){
        IEC1bits.U2TXIE=1;
        DMA0CONbits.CHEN=0;
        LATAbits.LATA5 = cambio;
        estadoUART = 3;
        data = 0;
        indice = 0;
        IFS1bits.U2TXIF=0;
    }
    
    IFS0bits.DMA0IF=0;
} 