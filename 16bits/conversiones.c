/*
Fichero: conversiones.c

 Descripción
 * Funciones de conversion
 */

// Conversion 1024 -> Dec10
unsigned char tabla_carac[10]="0123456789";
void conversion_tiempo (unsigned char *dir1, unsigned char *dir2, unsigned int val)
{
    unsigned char dig;
    
    dig=val/10;
    dig=tabla_carac[dig];
    *dir1=dig;
    
    dig=val%10;
    dig=tabla_carac[dig];
    *dir2=dig;
}

// Conversion 1024 -> Hex100
const unsigned char tabla_hex[16]="0123456789ABCDEF";
void Conversion_CAD (unsigned char *dir, unsigned int val)
{
    unsigned int d2,d1,d0;
    
    d2=val&0x0300;
    d2=d2>>8;
    *dir=tabla_hex[d2];
    dir++;
    d1=val&0x00F0;
    d1=d1>>4;
    *dir=tabla_hex[d1];
    dir++;
    d0=val&0x000F;
    *dir=tabla_hex[d0];
}

// Conversion 1024 -> Dec100
void Conversion_PWM(unsigned char *dir, unsigned int val){

     unsigned char dig;
    
    dig=val/1000;
    dig=tabla_carac[dig];
    *dir=dig;
    val %= 1000;
    dir++;
    
    dig=val/100;
    dig=tabla_carac[dig];
    *dir=dig;
    val %= 100;
    dir++;
    
    dig=val/10;
    dig=tabla_carac[dig];
    *dir=dig;
    dir++;
    
    dig=val%10;
    dig=tabla_carac[dig];
    *dir=dig;
}
