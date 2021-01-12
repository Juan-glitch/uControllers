/* 
* Fichero: TRAP.c
*
* Define las funciones de atención a las interrupciones tipo TRAP 
* No hace falta habilitarlas 
*/

#include <p24HJ256GP610A.h>

#define _ISR_NO_PSV __attribute__((interrupt, no_auto_psv))

void _ISR_NO_PSV _OscillatorFail(void)
{
    Nop();
    Nop();
    while (1);
} 

void _ISR_NO_PSV _AddressError(void)
{
    Nop();
    Nop();
    while (1);
} 

void _ISR_NO_PSV _StackError (void)
{
    Nop();
    Nop();
    while (1);
} 

void _ISR_NO_PSV _MathError (void)
{
    Nop();
    Nop();
    while (1);
} 

void _ISR_NO_PSV _DefaultInterrupt (void)
{
    Nop();
    Nop();
    while (1);
} 

void _ISR_NO_PSV _DMACError(void)
{
    Nop();
    Nop();
    while (1);  
}
