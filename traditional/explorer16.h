/* 
Proyecto:
Fichero: explorer16.h

Definiciones generales
*/
  
#include <p24HJ256GP610A.h>

#define _ISR_NO_PSV __attribute__((interrupt, no_auto_psv)) 

#define TRIS_LED_D3 TRISAbits.TRISA0
#define TRIS_LED_D4 TRISAbits.TRISA1
#define TRIS_LED_D5 TRISAbits.TRISA2
#define TRIS_LED_D6 TRISAbits.TRISA3
#define TRIS_LED_D7 TRISAbits.TRISA4
#define TRIS_LED_D8 TRISAbits.TRISA5

#define LED_D3 LATAbits.LATA0
#define LED_D4 LATAbits.LATA1
#define LED_D5 LATAbits.LATA2
#define LED_D6 LATAbits.LATA3
#define LED_D7 LATAbits.LATA4
#define LED_D8 LATAbits.LATA5

#define Pulsador_S3_TRIS TRISDbits.TRISD6
#define Pulsador_S6_TRIS TRISDbits.TRISD7
#define Pulsador_S5_TRIS TRISAbits.TRISA7
#define Pulsador_S4_TRIS TRISDbits.TRISD13

#define START_S3 PORTDbits.RD6
#define STOP_S6 PORTDbits.RD7
#define RESET_S5 PORTAbits.RA7
#define SCROLL_S4 PORTDbits.RD13

#define XJOY_TRIS TRISBbits.TRISB0
#define YJOY_TRIS TRISBbits.TRISB1

#define XJOY PORTBbits.RB0
#define YJOY PORTBbits.RB1

#define TEMP_TRIS TRISBbits.TRISB4
#define POT_TRIS TRISBbits.TRISB5

#define TEMP PORTBbits.RB4
#define POT PORTBbits.RB5

// PWM
#define PULSO_0 0
#define PULSO_1 1
