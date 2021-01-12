#Makefile example for PIOs (Leds, Wwitches and Keys)
# G.A.- MISE- ATC/KAT (UPV/EHU)


obj-m += DisplayModule.o

ModuloDisplay-objs := DisplayModule.o 

obj-m += PIOModule.o

ModuloPIO-objs := PIOModule.o 

obj-m += MotorModule.o

ModuloPIO-objs := MotorModule.o 

obj-m += AcelModule.o

ModuloPIO-objs := AcelModule.o 


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm menu


menu: menu.c
	gcc -o menu menu.c

