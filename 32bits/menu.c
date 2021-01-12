///
///	@file menu.c 
///	@brief Este fichero C contiene el codigo fuente de la aplicacion.
///	       Esta aplicacion interactua con los modulos kernel desarrollados.
///        Mediante esta aplicacion se controla el HW de la placa altera.
///
///	@author A.Cabrera y J.Arin
///
///	@date 2020/12/13
///

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>

#define TAMBUF 32    // límite lectura/escritura

#include "../address_map_arm.h" 

#define RAD_TO_GRAD 57.29577951

int segmentos ( int number);
int ctoi (char ch);
void inicializar_altera(int canal_1, int canal_2, int canal_4);

int write_altera(int fd, void * buffer, int size);
int read_altera(int fd, void * buffer, int size);

int main(int argc, char *argv[])
{
    int option, leds,display,param, Duty_M1, Duty_M2, Duty_general, Grados, Eje, cont,SW_eje_X,SW0,SW1,temp;
	long Eje_X, Eje_Y;
	double x, A_x,ret, val, dato_x, Rot_X, Rot_Y;
	double G_X, G_Y, grados_x, grados_y;
	int32_t Grad_M2, Grad_M1;
    char buf[TAMBUF];
    char Disp_1[] = "/dev/myDisplay";
	char Disp_2[] = "/dev/myPIO";
	char Disp_3[] = "/dev/myMOTOR";
	char Disp_4[] = "/dev/myACEL";
    int LEDS[] = {0x0,0x1,0x3,0x7,0xF,0x1F,0x3F,0x7F,0xFF,0x1FF,0x3FF};
    int fd = -1;   
    int Disp, minus;
    int index, SW_value, SW_enable, LED_value;

    int canal_1, canal_2, canal_3, canal_4;
	
	canal_1 = open(Disp_1, O_RDWR);
	canal_2 = open(Disp_2, O_RDWR);
	canal_3 = open(Disp_3, O_RDWR);
	canal_4 = open(Disp_4, O_RDWR);
	
	inicializar_altera(canal_1,canal_2,canal_4);
		
	do{
		system("clear");
		printf(" \n            Modo Manual\n");
		printf("--------------------------------------\n");
		printf(" 1. LED Manual\n");
		printf(" 2. SW Manual\n");
		printf(" 3. DISPLAY Manual\n");
		printf(" 4. Visualizar Motores\n");
		printf(" 5. Controlar Motores\n");
		printf(" 6. Visualizar Acelerometro\n");
		printf(" 7. Mover Motores con Acelerometro\n");
		printf(" 8. Salir\n");
		printf("\nElige una opción (1-8): ");
		
		fgets(buf,sizeof(buf),stdin);
		sscanf(buf,"%d",&option);
		
		switch(option){
		
			
			case 1: // LED Manual
				printf("\n  Indica el número (0..10) de LEDs a encender:   ");
				
				//Recoger valor de la terminal
				fgets(buf,sizeof(buf),stdin);
				sscanf(buf,"%d",&leds);
				
				if(0 <= leds && leds <= 10){
					
					// Encender Leds
					write_altera(canal_2, &LEDS[leds], sizeof(LEDS[leds]));
				}
			
				break;

			
			case 2: //SW Manual	

				printf("\n FUNCIONAMIENTO:\n\n");
				printf("\t>> Para empezar la interacion con los SW mantener encendido el interruptor SW9\n");
				
				// Encender Led superior a SW9
				LED_value = 0x200;
				write_altera(canal_2, &LED_value, sizeof(LED_value));
				
				
				do{  // Esperar hasta que el usuario encienda el SW9
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200; 
				}while(!SW_enable);
				
				printf("\t>> Ahora puede encender y apagar los interruptores\n");
				printf("\t>> Para terminar apagar el interruptor SW9\n\n");
				
				do{  //Encender el led superior al SW encendido
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					write_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200;
				}while(SW_enable);
				
				
				
				printf(" Apagar todos los SW...\n");
					
				do{
					read_altera(canal_2, &SW_value, sizeof(SW_value));
				}while(SW_value);

				SW_enable = 0;
				LED_value = 0;
				write_altera(canal_2, &LED_value, sizeof(LED_value));
				
				break;	

									
			case 3: //DISPLAY Manual

				printf("\n  Introduce un número de 0 a 9999:   ");
				
				//Recoger valor de la terminal
				fgets(buf,sizeof(buf),stdin);
				sscanf(buf,"%d",&param);

				//Escribir en el display (HEX3_HEX0)
				Disp = segmentos (param);
				write_altera(canal_1, &Disp, sizeof(Disp));
				
				
				break;

				
			case 4:  // Visualizar Motores
			
				printf("\n\n FUNCIONAMIENTO:\n\n");
				printf("\t>> Para empezar encender el interruptor SW9\n");
				printf("\t>> Y para terminar apagar el interruptor SW9\n");

				// Encender Led superior a SW9
				LED_value = 0x200;
				write_altera(canal_2, &LED_value, sizeof(LED_value));
				
				do{ // Esperar hasta que el usuario encienda el SW9
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200; 
				}while(!SW_enable);
				
				printf("\n\t>> SW0 encendido: Visualizar M1 en display\n");
				printf("\t>> SW0 apagado: Visualizar M2 en display\n\n");
				
				SW_eje_X = 0;
				
				do{
		
					// Leer valor de los SWs
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200;
					SW0 = SW_value & 0x1;
					
					// Leer DUTY CYCLE de los motores
					read_altera(canal_3, &Duty_general, sizeof(Duty_M1));
					Duty_M1 = Duty_general & 0xFFFF;
					Duty_M2 = (Duty_general & 0xFFFF0000) >> 16;

					// Conevertir el duty cycle en grados
					Grad_M1 = (Duty_M1 * 10) - 50;
					Grad_M2 = (Duty_M2 * 10) - 50;

					if(SW0) {temp = Grad_M1;}
					else {temp = Grad_M2;}
					
					// Visualizar grados en el display
					Disp = segmentos (temp);
					write_altera(canal_1, &Disp, sizeof(Disp));
					
					// Visualizar grados en el terminal
					printf("\r\tM2: %d   M1: %d", Grad_M2, Grad_M1);
					
				}while(SW_enable);
				
				printf("\n");
				SW_enable = 0;
				LED_value = 0;
				Disp = 0;
				write_altera(canal_1, &Disp, sizeof(Disp));
				write_altera(canal_2, &LED_value, sizeof(LED_value));
				
				break;
			
			case 5:  //Controlar Motores
			
				printf("\n\n FUNCIONAMIENTO:\n\n");
				printf("\t>> Para empezar encender el interruptor SW9\n");
				printf("\t>> Y para terminar apagar el interruptor SW9\n");

				// Encender Led superior a SW9
				LED_value = 0x200;
				write_altera(canal_2, &LED_value, sizeof(LED_value));
				
				do{ // Esperar hasta que el usuario encienda el SW9
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200; 
				}while(!SW_enable);
				
				printf("\n\t>> SW0 encendido: Introducir los grados M1 manualmente\n");
				printf("\t>> SW1 encendido: Introducir los grados M2 manualmente\n\n");
				
				SW_eje_X = 0;
				
				do{
		
					// Leer valor de los SWs
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200;
					SW0 = SW_value & 0x1;
					SW1 = SW_value & 0x2;

					if(SW0){  // Control Motor 1
					
						printf("\r\tIndica la posicion del Motor 1 (0 a 180): ");
						
						//Recoger valor de la terminal
						fgets(buf,sizeof(buf),stdin);
						sscanf(buf,"%d",&param);
						
						if(param <= 180){

							// Convertir los grados a Duty Cycle para el motor
							temp = (param + 50) / 10;
							Duty_M1 = temp;
							
							// Escribir en el DUTY CYCLE del motor M1
							Duty_general = (Duty_general & 0xFFFF0000) + Duty_M1;
							write_altera(canal_3, &Duty_general, sizeof(Duty_general));
						}
						

						
					}
					else{
					
						printf("\r\tIndica la posicion del Motor 2 (0 a 180): ");
						
						fgets(buf,sizeof(buf),stdin);
						sscanf(buf,"%d",&param);
						
						if(param <= 180){

							// Convertir los grados a Duty Cycle para el motor
							temp = (param + 50) / 10;
							Duty_M2 = (temp) << 16;
					
							// Escribir en el DUTY CYCLE del motor M1
							Duty_general = (Duty_general & 0xFFFF) + Duty_M2;
							write_altera(canal_3, &Duty_general, sizeof(Duty_general));
						}
						
					}
					
					// Leer el duty cycle de los motores
					read_altera(canal_3, &Duty_general, sizeof(Duty_M1));
					Duty_M1 = Duty_general & 0xFFFF;
					Duty_M2 = (Duty_general & 0xFFFF0000) >> 16;

					// Convertir el Duty Cycle en grados
					Grad_M1 = (Duty_M1 * 10) - 50;
					Grad_M2 = (Duty_M2 * 10) - 50;

					if(SW0) {temp = Grad_M1;}
					else {temp = Grad_M2;}
					
					// Visualizar los grados en el display
					Disp = segmentos (temp);
					write_altera(canal_1, &Disp, sizeof(Disp));
					
				}while(SW_enable);
				
				printf("\n");
				SW_enable = 0;
				LED_value = 0;
				Disp = 0;
				write_altera(canal_1, &Disp, sizeof(Disp));
				write_altera(canal_2, &LED_value, sizeof(LED_value));
				
				break;
				
				
			case 6:  //Visualizar Acelerometro
				
				printf("\n\n FUNCIONAMIENTO:\n\n");
				printf("\t>> Para empezar encender el interruptor SW9\n");
				printf("\t>> Y para terminar apagar el interruptor SW9\n");
				LED_value = 0x200;
				write_altera(canal_2, &LED_value, sizeof(LED_value));
				
				do{
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200; 
				}while(!SW_enable);
				
				printf("\n\t>> En el display se muestran los GRADOS: \n");
				printf("\t>> Para cambiar de eje encender el interruptor SW0\n");
				
				do{
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200;
					SW_eje_X = SW_value & 0x1; 

					// Leer el Ton del acelerometro (Eje X y Eje Y)
					read_altera(canal_4, &Eje, sizeof(Eje));
					
					Eje_X = Eje & 0xFFFF;
					Eje_Y = (Eje & 0xFFFF0000) >> 16;

					// Convertir el Ton en g
					G_X= ((Eje_X / 100.0) - 0.5) * 8;
					G_Y = ((Eje_Y / 100.0) - 0.5) * 8;
					
					// Calcular los grados
					grados_x = asin(G_X) * RAD_TO_GRAD;
					grados_y = asin(G_Y) * RAD_TO_GRAD;

					usleep(1000);

					// Visualizar los grados en el display
					if(SW_eje_X){
						if(Eje_X > 1){
							Disp = 0x76; //X
							write_altera(canal_4, &Disp, sizeof(Disp));
							Disp = segmentos((int)grados_x);
							write_altera(canal_1, &Disp, sizeof(Disp));
						
							if(Eje_X < 47){
								minus = (0x40) << 16;
								Disp = Disp | minus;
								write_altera(canal_1, &Disp, sizeof(Disp));
							}
						}
					}
					else{
						if(Eje_Y > 1){
						Disp = 0x6E; //Y
						write_altera(canal_4, &Disp, sizeof(Disp));
						Disp = segmentos((int)grados_y);
						write_altera(canal_1, &Disp, sizeof(Disp));
						if(Eje_Y < 47){
								minus = (0x40) << 16;
								Disp = Disp | minus;
								write_altera(canal_1, &Disp, sizeof(Disp));
							}
						}
					}
					
					
				}while(SW_enable);
				
				inicializar_altera(canal_1,canal_2,canal_4);
				
				break;
			
			case 7:  //Mover Motores con Acelerometro

				printf("\n\n FUNCIONAMIENTO:\n\n");
				printf("\t>> Para empezar encender el interruptor SW9\n");
				printf("\t>> Y para terminar apagar el interruptor SW9\n");
				LED_value = 0x200;
				write_altera(canal_2, &LED_value, sizeof(LED_value));
				
				do{
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200; 
				}while(!SW_enable);

				
				do{
					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_enable = SW_value & 0x200;
					
					read_altera(canal_4, &Eje, sizeof(Eje));
					
					Eje_X = Eje & 0xFFFF;
					Eje_Y = (Eje & 0xFFFF0000) >> 16;

					read_altera(canal_2, &SW_value, sizeof(SW_value));
					SW_eje_X = SW_value & 0x1; 

					usleep(2000);

					// Leer los valores del acelerometro y convertirlos en Duty Cycle para los motores
					if(SW_eje_X){
						if(Eje_X > 1){
							Grad_M1 = (9 * Eje_X) - 360;
							if((0 <= Grad_M1) && (Grad_M1 <= 180)){

								temp = (Grad_M1 + 50) / 10;
								Duty_M1 = temp;

								Disp = segmentos(temp);
								write_altera(canal_1, &Disp, sizeof(Disp));

								Disp = 0x76; //X
								write_altera(canal_4, &Disp, sizeof(Disp));

								Duty_general = (Duty_general & 0xFFFF0000) + Duty_M1;
								write_altera(canal_3, &Duty_general, sizeof(Duty_general));
							}
						}
					}
					else{
						if(Eje_Y > 1){
							Grad_M2 = (9 * Eje_Y) - 360;
							if( (0 <= Grad_M2) && (Grad_M2 <= 180)){

								temp = (Grad_M2 + 50) / 10;
								Duty_M2 = (temp) << 16;

								Disp = segmentos(temp);
								write_altera(canal_1, &Disp, sizeof(Disp));

								Disp = 0x6E; //Y
								write_altera(canal_4, &Disp, sizeof(Disp));

								Duty_general = (Duty_general & 0xFFFF) + Duty_M2;
								write_altera(canal_3, &Duty_general, sizeof(Duty_general));
							}
						}
					}
				}while(SW_enable);
				
				inicializar_altera(canal_1,canal_2,canal_4);
				break;
				
			case 8: //Salir
				inicializar_altera(canal_1,canal_2,canal_4);
				break;	

			default:
				printf("\n**** Opción no válida **** \n");
				sleep(1);
				break;
																
		}
		
	}while (option != 8);
				
				
    exit(0);
}


void inicializar_altera(int canal_1, int canal_2, int canal_4)
{
	int LED_value = 0, Disp = 0, SW_value;

	write_altera(canal_2, &LED_value, sizeof(LED_value));
	write_altera(canal_1, &Disp, sizeof(Disp));
	write_altera(canal_4, &Disp, sizeof(Disp));
	read_altera(canal_2, &SW_value, sizeof(SW_value));

	if(SW_value) printf(" Apagar todos los SW...\n");						
	do{
		read_altera(canal_2, &SW_value, sizeof(SW_value));
	}while(SW_value);

}


int write_altera(int fd, void * buffer, int size)
{
	int n;
	
	if ((n = write (fd, buffer, size))!= size) {
		printf("No se ha podido escribir el dispositivo Ret=%d\n",n );
		close(fd);
		exit(3);
	}
	return(n);

}

int read_altera(int fd, void * buffer, int size)
{
	int n;
	
	if ((n = read(fd, buffer, size))!= size) {
		printf("No se ha podido escribir el dispositivo Ret=%d\n",n );
		close(fd);
		exit(3);
	}
	return(n);

}

/* SEGMENTOS: 
   DEF: codificar número decimal para 7-segment displays
   IN: numero
   OUT = valor 4 registro display*/
   int segmentos ( int number) {
   char digitos[5]={'\0'};
   int c, i;
   int SEG_HEX[]={0x3F, 0x06, 0x5B, 0x4F,0x66,0x6D, 0x7D, 0x07, 0x7F, 0xEF}; 

   sprintf(digitos,"%d",number);  
   c = 0x0;
   for (i=0;i<strlen(digitos);i++){
                c = c << 8 | SEG_HEX[ctoi(digitos[i])];
   }
   return c;
}

/* ctoi: 
   DEF: codificar CARACTER recibido a integer
   IN: caracter
   OUT = Numero*/
int ctoi (char ch){
        if (ch >= '0' && ch <= '9' ) return ch - '0';
        return -1; //error
}
