/*Fichero: memoria.c
 Descripción
 * Funciones para la gestion de constantes en memoria
*/

unsigned char Ventana_START[2][18]=
{
    {"  Proyecto 2021 ""\x0D\x0A"}, //0
    {"--- Pulsa S3 ---""\x0D\x0A"}, //1
};

//LCD
//al final de cada linea CR (0x0D) y LF (0x0A)
unsigned char Ventana_LCD[10][18]=
{
    {"Proyecto MISElcd""\x0D\x0A"}, //0
    {"Crono.: --:--,- ""\x0D\x0A"}, //1
    {"== INFO PIC24H =""\x0D\x0A"}, //2
    {"Poten.:      ---""\x0D\x0A"}, //3
    {"Tenperatura: ---""\x0D\x0A"}, //4
    {"Joystick X:  ---""\x0D\x0A"}, //5
    {"Joystick Y:  ---""\x0D\x0A"}, //6
    {"Duty PWM:   ----""\x0D\x0A"}, //7
    {"CPU (libre): --%""\x0D\x0A"}, //8
    {"Max: -- Min: --%""\x0D\x0A"}, //9
};

//Intervalos de Ventana_LCD para el scroll
unsigned char lcd1,lcd2;

//UART por DMA
unsigned char Ventana_LCD_DMA[10][19] __attribute__((space(dma))) =
{
    {"\x1b[H""Proyecto MISEdma"}, //0
    {"\x0d\x0a""Crono.: --:--,-  "}, //1
    {"\x0d\x0a""== INFO PIC24H =="}, //2
    {"\x0d\x0a""Poten.:       ---"}, //3
    {"\x0d\x0a""Tenperatura:  ---"}, //4
    {"\x0d\x0a""Joystick X:   ---"}, //5
    {"\x0d\x0a""Joystick Y:   ---"}, //6
    {"\x0d\x0a""Duty PWM:    ----"}, //7
    {"\x0d\x0a""CPU (libre):  --%"}, //8
    {"\x0d\x0a""Max: --  Min: --%"}, //9
};


//CAD por DMA
unsigned int CAD_BuffA[32] __attribute__((space(dma)));
unsigned int CAD_BuffB[32] __attribute__((space(dma)));