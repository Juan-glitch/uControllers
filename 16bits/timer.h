/*
Fichero: timer.h

 Descripción
 * Variables y cabeceras del modulo timer.c
 */

void Inicializar_T1 ();
void Inicializar_T3 ();
void Inicializar_T4 ();
void Inicializar_T2_LCD ();
void Inicializar_T5 ();  // 20ms
void Inicializar_T6 ();
void Inicializar_crono();
void Cronometro();

void Delay_T4_us (unsigned int nus);
void Delay_T4_ms (unsigned int nms);

extern unsigned int scroll;
