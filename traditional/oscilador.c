/*
* Fichero: oscilador.c
*
* Programa el PLL para la frecuencia del Oscilador Principal a 80MHz
*/

#include <p24HJ256GP610A.h>

/*
**	Oscillator Mode: Register FOSCSEL
**     FNOSC_FRC            Internal Fast RC (FRC)
**     FNOSC_FRCPLL         Internal Fast RC (FRC) con PLL
**     FNOSC_PRI            Primary Oscillator (XT, HS, EC)
**     FNOSC_PRIPLL         Primary Oscillator (XT, HS, EC) con PLL
**     FNOSC_SOSC           Secondary Oscillator (SOSC)
**     FNOSC_LPRC           Low Power RC Oscillator (LPRC)
**     FNOSC_FRCDIV16       Internal Fast RC (FRC) divide by 16
**     FNOSC_LPRCDIVN       Internal Fast RC (FRC) with divide by N
**	Primary Oscillator Source: Register FOSC
**     POSCMD_EC            EC Oscillator Mode
**     POSCMD_XT            XT Oscillator Mode
**     POSCMD_HS            HS Oscillator Mode
**     POSCMD_NONE          Primary Oscillator Disabled
**   OSC2 Pin Function:
**     OSCIOFNC_ON          OSC2 pin has digital I/O function
**     OSCIOFNC_OFF         OSC2 pin has clock out function
**   Clock Switching and Monitor:
**     FCKSM_CSECME         Both Clock Switching and Fail-Safe Clock Monitor are enabled
**     FCKSM_CSECMD         Clock switching is enabled, Fail-Safe Clock Monitor is disabled
**     FCKSM_CSDCMD         Both Clock Switching and Fail-Safe Clock Monitor are disabled
**
*  Para ver configuración: Window -> PIC Memory Views -> Configuration Bits
*/

//=========================================================================
// Estas PRAGMA se ejecutan al grabar el programa
#pragma config FNOSC = PRIPLL  	// Oscillator Primario (XT, HS, EC) con PPL
#pragma config FCKSM = CSECMD   // Only clock switching enabled
#pragma config OSCIOFNC = OFF   // OSC2 is clock O/P
#pragma config POSCMD = XT      // XT oscillator    

#pragma config FWDTEN = OFF     // Watchdog Timer: Disabled
// NOTA: FWDTEN no tiene nada que ver con el oscilador
//===========================================================================

void Inic_Oscilador ()
{
// Configura el Oscilador para operar a x MHz
// Fosc= Fin*M(N1*N2),  Fcy=Fosc/2
// Fosc= 8*40/(2*2)= 80 MHz for 8 MHz input clock
// M=40 --> oscilador 80 MHz Instruccion 25ns
// M=20 --> oscilador 40 MHz Instruccion 50ns
// M=10 --> oscilador 20 MHz Instruccion 100ns

	PLLFBD = 40-2;          // M
	CLKDIVbits.PLLPOST = 0; // N2
	CLKDIVbits.PLLPRE = 0;  // N1

// Clock switch to incorporate PLL
	__builtin_write_OSCCONH(0x03);		// Initiate Clock Switch to Primary
																		// Oscillator with PLL (NOSC=0b011)
																		
	__builtin_write_OSCCONL(0x01);		// Start clock switching
	
// Wait
	while(OSCCONbits.COSC!=0b011);	// Wait fot Clock switch to occur
	while(OSCCONbits.LOCK!=1);			// Wait for PLL to lock
}
