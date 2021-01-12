///
///	@file MotorModule.c 
///	@brief Este fichero C contiene un módulo de kernel Linux
///	       que gestiona los dos motores y los KEYs por interrupcion.Ofrece dos funciones de read y write.
///                READ: Lee el DUTY CYCLE de los motores
///                WRITE: Escribe el DUTY CYCLE de los motores
///
///	@author A.Cabrera y J.Arin
///
///	@date 2020/12/13
///

#include <linux/module.h>	// included for all kernel modules
#include <linux/kernel.h>	// included for KERN_INFO
#include <linux/init.h>     // included for __init and __exit macros
#include <linux/interrupt.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timex.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include "../address_map_arm.h"
#include "../interrupt_ID.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("G.Alvarez");
MODULE_DESCRIPTION("Sample read/write KEY-LEDS-SW driver");

#define KEYS_DataReg	 	0
#define KEYS_DirectionReg 	1
#define KEYS_IRQEnable 		2
#define KEYS_EdgeCapReg 	3

/// Proptotipe motor_read function 
static ssize_t motor_read(struct file *, char *, size_t, loff_t *);


/// Proptotipe motor_write function
static ssize_t motor_write(struct file *, const char *, size_t , loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "myMOTOR"

static dev_t dev_no = 0;
static struct cdev *myMOTORdev_cdev = NULL;
static struct class *myMOTORdev_class = NULL;

static uint32_t DataRW;

void * LW_virtual;                              // Lightweight bridge Virtual base address
void *HPS_TIMER2_virtual;                       // Virtual address for HPS_TIMER2


volatile uint32_t *LEDR_ptr;                    // virtual address for the LEDR port

volatile uint32_t *JP2_ptr;                     // virtual address for the JP2 port
volatile uint32_t *JP2_Data;                    // virtual address for the JP2 Data Register 
volatile uint32_t *JP2_Direction;               // virtual address for the JP2 Direction Register 
volatile uint32_t *JP2_Imask;                   // virtual address for the JP2 Interrupt Mask Register 

volatile uint32_t *KEY_ptr;                     // virtual address for the KEY port

volatile uint32_t *HPS_Timer2_ptr;              // virtual address for the HPS_Timer2 Controller
volatile uint32_t *HPS_TIMER2_Control;          // virtual address for the HPS_TIMER2 Control Register
volatile uint32_t *HPS_TIMER2_Load;             // virtual address for the HPS_TIMER2 Load Register
volatile uint32_t *HPS_TIMER2_EOI;              // virtual address for the HPS_TIMER2 EOI Register

volatile uint32_t Duty_general = 0x00050005;    // Initialice with DUTYMIN (5) M1 and M2

#define T20ms        200                        // 20 ms
#define DUTYMAX      23                         // 2300 us
#define DUTYMIN      5                          // 500 us

volatile uint32_t Duty_M1 = 5, Duty_M2 = 5, Ton_M1 = 0, Toff_M1 = 0, Ton_M2 = 0, Toff_M2 = 0, Duty_tmp;

#define KEYS_DataReg	 	0
#define KEYS_DirectionReg 	1
#define KEYS_IRQEnable 		2
#define KEYS_EdgeCapReg 	3


/*
///	This function  will be used to configure the JP2 port
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
void config_JP2(void){

        // Reset JP2 data register
        *JP2_Data = 0; 
        
        // Configure D5 and D7 as output
        *JP2_Direction = 0xA0;
        
        // Disable Interrupt
        *JP2_Imask = 0;

}


/*
///	This function  will be used to configure HPS TIMER2
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
void config_TIMER2_HPS (void){

	//Stop Timer2
	*HPS_TIMER2_Control = 0b0;

	//Configure TIMER2 Load REgister to 100 usec
	*HPS_TIMER2_Load = 2500;   // 25 MHz -> 100 us 

	//Configure TIMER2 Mode Register 
	*HPS_TIMER2_Control = 0b011;	//Interrupt enable, Autoreload enable, Timer enable
	// Mask == 0 Enable Interrupt
	// Mask == 1 Disable Interrupt
}


/*
///	This function  is the ISR of the KEYs. Will be used to manage the interruption of the KEYs. 
///     It will be used to increase or decrease the Duty Cycle of the Motors.
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
irq_handler_t KEY_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{

        uint32_t Key;

        Key = *(KEY_ptr + KEYS_EdgeCapReg);
        printk(KERN_INFO "Duty: %d\n",Duty_M2);

        // Si estamos pulsando KEY0 disminuir el Duty Cycle de M1
        if ((Key & 0x1) && (Duty_M1 > DUTYMIN)) {

                Duty_M1 = Duty_M1 - 1;
                Duty_general = (Duty_general & 0xFFFF0000) + Duty_M1;
        }

        // Si estamos pulsando KEY1 aumentar el Duty Cycle de M1
        if ((Key & 0x2) && (Duty_M1 < DUTYMAX)){
	       
	        Duty_M1 = Duty_M1 + 1;
	        Duty_general = (Duty_general & 0xFFFF0000) + Duty_M1;
        }

        // Si estamos pulsando KEY2 disminuir el Duty Cycle de M2
        if ((Key & 0x4) && (Duty_M2 > DUTYMIN)){
	        
                Duty_M2 = Duty_M2 - 1;
                Duty_tmp = (Duty_M2) << 16;
                Duty_general = (Duty_general & 0xFFFF) + Duty_tmp;
        }

        // Si estamos pulsando KEY3 aumentar el Duty Cycle de M2
        if ((Key & 0x8) && (Duty_M2 < DUTYMAX)){
	  
	        Duty_M2 = Duty_M2 + 1;
	        Duty_tmp = (Duty_M2) << 16;
	        Duty_general = (Duty_general & 0xFFFF) + Duty_tmp;
        }

        // Clear the edgecapture register (clears current interrupt)
        *(KEY_ptr + KEYS_EdgeCapReg) = 0xF;

        return (irq_handler_t) IRQ_HANDLED;
}


/*
///	This function  is the ISR of the TIMER2. Will be used to manage the interruption of the TIMER2. 
///     It will be used to manage the Ton and Toff times of the Motors.
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
irq_handler_t hps_timer2_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{

        //Every 100 us the timer execute this rutine, so if we want Ton = 600us, Duty_M1 is going to be 6
        
        // Ton and Toff time manage for M1
        if(Ton_M1 < Duty_M1){

	        *JP2_Data = *JP2_Data | 0x80;   // Encender
	        Ton_M1++;
        }
        else {

	        *JP2_Data = *JP2_Data &~ 0x80;   // Apagar
	        Toff_M1++;
	        
	        if((Ton_M1 + Toff_M1) == T20ms){
                        Ton_M1 = 0;
                        Toff_M1 = 0;
                        Duty_M1 = Duty_general & 0xFFFF;
	        }
        }

        // Ton and Toff time manage for M2
        if(Ton_M2 < Duty_M2){

	        *JP2_Data = *JP2_Data | 0x20;   // Encender
	        Ton_M2++;
        }
        else {

	        *JP2_Data = *JP2_Data &~ 0x20;   // Apagar
	        Toff_M2++;
	        
	        if((Ton_M2 + Toff_M2) == T20ms){
		        Ton_M2 = 0;
		        Toff_M2 = 0;
		        Duty_M2 = (Duty_general & 0xFFFF0000) >> 16;
	        }
        }

        // Clear the EOI register (clears current interrupt)
        *(HPS_TIMER2_EOI); 	// EOI (Read Timer2 EOI register)

        return (irq_handler_t) IRQ_HANDLED;
}


int initializeKeys(void)
{
        int value;

        // Clear the PIO edgecapture register (clear any pending interrupt)
        *(KEY_ptr + KEYS_EdgeCapReg) = 0xF;
        
        // Enable IRQ generation for the 4 buttons
        *(KEY_ptr + KEYS_IRQEnable) = 0xF;

        // Register the interrupt handler.
        value = request_irq (KEYS_IRQ, (irq_handler_t) KEY_irq_handler, IRQF_SHARED, 
        "KEY irq handler", (void *) (KEY_irq_handler));

        return value;
}





/// Module operations Vector for Motor
static struct file_operations fopsMotor = {
	 .owner = THIS_MODULE,
	 .read = motor_read,
	 .write = motor_write
};

	
/**
*	This function  is the Linux module init Function 
*	@return <>0 on ERROR  (0 on OK)
*	@author G. Alvarez
*	@date 11/23/2018
*/
static int __init init_PIOdev(void){

        int err = 0, value;

        /// allocate the mayor and minor numbers for PIO device
        if ((err = alloc_chrdev_region (&dev_no, 0, 1, DEVICE_NAME)) < 0) {
	        return err;
        }

        /// Createthe class for  device
        myMOTORdev_class = class_create (THIS_MODULE, DEVICE_NAME);

        /// Allocate and initialize the char device
        myMOTORdev_cdev = cdev_alloc ();
        myMOTORdev_cdev->ops = &fopsMotor;
        myMOTORdev_cdev->owner = THIS_MODULE;

        /// myMOTORdev cdev registration (on kernel)
        if ((err = cdev_add (myMOTORdev_cdev, dev_no, 1)) < 0) {
	        return err;
        }
        device_create (myMOTORdev_class, NULL, dev_no, NULL, DEVICE_NAME );

        // Call to generate a virtual address region for the FPGA lightweight bridge Devices
        LW_virtual = ioremap_nocache (LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

        // Call to generate a virtual address region for the HPS Timer2
        HPS_TIMER2_virtual = ioremap_nocache (HPS_TIMER2 , HPS_TIMER2_SPAN);

        // Initialize LEDR
        LEDR_ptr = (uint32_t *) (LW_virtual + LEDR_BASE);       // init virtual address for LEDR port
        *LEDR_ptr = 0x200;                                      // turn on the leftmost light

        JP2_ptr = (uint32_t *) (LW_virtual + JP2_BASE);         // init virtual address for LEDR port
        JP2_Data        =  JP2_ptr + 0;                         // init virtual address for JP2 Data Register
        JP2_Direction   =  JP2_ptr + 1;                         // init virtual address for JP2 Direction Register
        JP2_Imask       =  JP2_ptr + 2;                         // init virtual address for JP2 Interrupt Mask Register

        // Initialize HPS Timer2
        HPS_Timer2_ptr       = (uint32_t *) HPS_TIMER2_virtual; // init virtual address for HPS_Timer2_ptr Device Registers
        HPS_TIMER2_Load      =  HPS_Timer2_ptr + 0;             // init virtual address for HPS Timer2 Load Register
        HPS_TIMER2_Control   =  HPS_Timer2_ptr + 2;             // init virtual address for HPS Timer2 Control Register
        HPS_TIMER2_EOI       =  HPS_Timer2_ptr + 3;             // init virtual address for HPS Timer2 EOI Register


        KEY_ptr = LW_virtual + KEY_BASE;    // init virtual address for KEY port

        // Configure JP2
        config_JP2();

        // Configure Timer2 
        config_TIMER2_HPS();

        // Init Keys 
        initializeKeys();

        // Register the interrupt handler.
        value = request_irq (HPS_TIMER2_IRQ, (irq_handler_t) hps_timer2_irq_handler, IRQF_SHARED, 
        "hps_timer2_irq_handler", (void *) (hps_timer2_irq_handler));

        printk(KERN_INFO "START %s Device\n", DEVICE_NAME);

        return 0;
}


	
/**
*	This function  is the Linux module exit Function 
*	@author G. Alvarez
*	@date 11/23/2018
*/
static void __exit stop_PIOdev(void){
	
        //Configure TIMER2 Mode Register (Disable Timer2)
        *HPS_TIMER2_Control = 0b000;	//Interrupt mask->0,Mode->0,Enable->0

        // DISABLE IRQ generation for the 4 buttons
        *(KEY_ptr + 2) = 0x0; 

        *LEDR_ptr = 0; // Turn off LEDs 

        *JP2_ptr = 0; // Turn off JP2

        // Important !!!! Mandatory Task!!! Free all configure IRQ
        free_irq (HPS_TIMER2_IRQ, (void*) hps_timer2_irq_handler);
        free_irq (KEYS_IRQ, (void*) KEY_irq_handler);


        device_destroy (myMOTORdev_class, dev_no);
        cdev_del (myMOTORdev_cdev);
        class_destroy (myMOTORdev_class);
        unregister_chrdev_region (dev_no, 1);
	 
}



/**
*	This function  is the Linux module read Function 
*	Called when a process reads from DUTY CYCLE of the motors, if it is ready. 
*	If not, sends the last data available
* 	@param filp 		This is the struct File.
* 	@param userbuffer 	This is the user buffer.
* 	@param length 		This is the number of bytes to read.
* 	@param offset 		This is the f_pos of srtuct_file to update.
*
* 	@return 		if >= 0 is the numbers of bytes readed. If <0 error
*
*	@author G. Alvarez
*	@date 11/23/2018
*/ 
static ssize_t motor_read(struct file *filp, char *userbuffer, size_t length, loff_t *offset){

        size_t bytes;
        size_t ret;

        DataRW = Duty_general;

        printk(KERN_INFO "READ:offset=%ld   Key_Val(dec):%d  Hex_Key(0x%0x)\n", 
	        (long)*offset, DataRW, DataRW);
	        
        bytes = sizeof(DataRW);
	        
        bytes = bytes > length ? length : bytes; // too much to send at once?

        if(bytes!=0) ret=copy_to_user (userbuffer, (void*) &DataRW, bytes);

        return bytes;

}



/**
*	This function  is the Linux module write function 
*	Called when a process writes to DUTY CYCLE
* 	@param filp 		This is the struct File.
* 	@param userbuffer 	This is the user buffer.
* 	@param length 		This is the number of bytes to write.
* 	@param offset 		This is the f_pos of srtuct_file to update.
*
* 	@return 		= 0 nothing was writen.
* 					>0 the numbers of bytes was writen.  
*					<0 an error occurred.
*
*	@author G. Alvarez
*	@date 11/23/2018
*/
static ssize_t motor_write(struct file *filp, const char *userbuffer, size_t length, loff_t *offset) {
	size_t bytes;
	unsigned int Data;	
 	ssize_t retval = -ENOMEM;
			
	bytes = sizeof(DataRW); 	/// el dato de LEDS es de 4 bytes
		
	bytes = bytes > length ? length : bytes; // too much to write at once?
	
	if(copy_from_user (&Data, userbuffer, bytes)) {
		retval = -EFAULT;
	}
	else {
		printk(KERN_INFO "WRITE:offset=%ld   Data(dec):%d Data(hex):%0x \n", 	(long)*offset, Data, Data);
		Duty_general = Data;
		retval = bytes;
	}
	
	return retval;

}

	

MODULE_LICENSE("GPL");
module_init (init_PIOdev);
module_exit (stop_PIOdev);
