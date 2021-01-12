/*
Fichero: memoria.h

Variables (extern) y cabeceras de las funciones del modulo memoria.c
*/

// variables
extern const unsigned char Mens_LCD_1[16],Mens_LCD_2[16];
extern const unsigned char Mens_LCD_3[16],Mens_LCD_4[16];
extern const unsigned char Mens_LCD_CAD[]; 

extern unsigned char Ventana_LCD[10][18];
extern unsigned char Ventana_START[2][18];

// funciones
void Copiar_FLASH_RAM (const unsigned char *texto, unsigned int i);


// -------- UART por DMA
extern const unsigned char Mens_LCD_3_DMA[19],Mens_LCD_4_DMA[19]; 
extern const unsigned char Mens_LCD_CAD_DMA[];

extern unsigned char Ventana_LCD_DMA[2][19] __attribute__((space(dma)));
void Copiar_FLASH_RAM_DMA (const unsigned char *texto, unsigned int i);


//------------ CAD DMA ORDER -------------
//CAD1 - DMA (tenp + pot + joyx + joyy / 8 datos por cada dispositivo)
extern unsigned int CAD_BuffA[32] __attribute__((space(dma)));
extern unsigned int CAD_BuffB[32] __attribute__((space(dma)));
