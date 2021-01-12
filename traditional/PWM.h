/*
Fichero: PWM.h

Variables y cabeceras de las funciones del modulo PWM.c
*/

//DEFINES Y VARIABLES
#define T20ms  12500 	// Periodo adecuado para conseguir 20 ms

extern unsigned int T_MINPWM;
extern unsigned int T_MAXPWM;
extern unsigned int estadoPWM,DUTY,refrescarPWM;

// FUNCIONES
void PrepararPWM ();
void ActualizarPWM ();
void Cambiar_DUTY (unsigned int valpot);



