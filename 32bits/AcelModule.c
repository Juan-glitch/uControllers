///
///	@file AcelModule.c 
///	@brief Este fichero C contiene un módulo de kernel Linux
///	       que gestiona el acelerometro y los diplays HEX5 y HEX4.
///            Ofrece dos funciones de read y write:
///                READ: Lee las señales X e Y del acelerometro
///                WRITE: Escribe en los displays HEX5 y HEX4
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

#include "../address_map_arm.h"
#include "../interrupt_ID.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("A.Cabrera y J.Arin");
MODULE_DESCRIPTION("Read/write LEDS-SW driver");

void * LW_virtual;         // Lightweight bridge Virtual base address
void *HPS_TIMER3_virtual;  //Virtual address for HPS_TIMER2

/// acel_read function 
static ssize_t acel_read(struct file *, char *, size_t, loff_t *); // Funcion para leer

/// display_write function
static ssize_t HEX5_HEX4_write(struct file *, const char *, size_t , loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "myACEL"

static dev_t dev_no = 0;
static struct cdev *myACELdev_cdev = NULL;
static struct class *myACELdev_class = NULL;

static uint32_t DataRW;

volatile uint32_t *LEDR_ptr;            // virtual address for the LEDR port

volatile uint32_t *JP2_ptr;             // virtual address for the JP2 port
volatile uint32_t *JP2_Data;            // virtual address for the JP2 Data Register
volatile uint32_t *JP2_Direction;       // virtual address for the JP2 Direction Register    
volatile uint32_t *JP2_Imask;           // virtual address for the JP2 Interrupt Mask Register             
volatile uint32_t *JP2_Edge;            // virtual address for the JP2 Edge Register 

volatile uint32_t *HPS_Timer3_ptr;      // virtual address for the HPS_Timer3 Port
volatile uint32_t *HPS_TIMER3_Counter;  // virtual address for the HPS_TIMER3 Counter Register
volatile uint32_t *HPS_TIMER3_Control;  // virtual address for the HPS_TIMER3 Control Register
volatile uint32_t *HPS_TIMER3_Load;     // virtual address for the HPS_TIMER3 Load Register
volatile uint32_t *HPS_TIMER3_EOI;      // virtual address for the HPS_TIMER3 EOI Register

volatile uint32_t contador_X;           // Variable for aceleroemter X port counter
volatile uint32_t contador_Y;           // Variable for aceleroemter Y port counter
volatile uint32_t Ton_X;                // Variable for aceleroemter X port Ton    
volatile uint32_t Ton_Y;                // Variable for aceleroemter Y port Ton   
volatile uint32_t Ton;                  // Variable for aceleroemter general Ton

volatile int * HEX5_HEX4_ptr;           // virtual address for the HEX5_HEX4 port 	

#define TIMER3_FREQ  25000000
#define TIMER3_LOAD  2500               // 100 us


/*
///	This function  will be used to configure the HPS TIMER 3
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
void config_TIMER3 (void){

	//Stop Timer2
	*HPS_TIMER3_Control = 0b0;

	//Configure TIMER2 Load REgister to 100 usec
	*HPS_TIMER3_Load = TIMER3_LOAD; 

	//Configure TIMER2 Mode Register 
	*HPS_TIMER3_Control = 0b011;	//Interrupt Enable, Autoreload Enable, Timer Enable
	// Mask == 0 Enable Interrupt
	// Mask == 1 Disable Interrupt
}


/*
///	This function  will be used to configure JP2 port D19 and D17 (Acelerometer)
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
void config_ACEL(void){

        //Configure D19 and D17 to input
	*JP2_Direction = 0xA0; 
	
	//Disable D19 and D17 Interrupt
	*JP2_Imask = 0x00000; 
	
}


/*
///	This function  is the ISR of the HPS TIMER 3. Will be used to manage the interruption of the timer. 
///     It will be used to measure the Ton of the X and Y ports of the Accelerometer.
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
irq_handler_t hps_timer3_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
        int flag_X, flag_Y;

        // Shift D19 and D17 bits 
        flag_X = (*JP2_Data & 0x80000) >> 19;
        flag_Y = (*JP2_Data & 0x20000) >> 17;

        // If X port is HIGH (during Ton) increase the counter, if is LOW (during Toff) reset the counter
        if(flag_X) {contador_X++;}
        else {
                Ton_X = contador_X;
                Ton = (Ton & 0xFFFF0000) + Ton_X;
                contador_X = 0;
        }

   
        // If Y port is HIGH (during Ton) increase the counter, if is LOW (during Toff) reset the counter
        if(flag_Y) {contador_Y++;}
        else {
                Ton_Y = (contador_Y) << 16;
                Ton = (Ton & 0xFFFF) + Ton_Y;
                contador_Y = 0;
        }
   

        // Clear the EOI register (clears current interrupt)
        *(HPS_TIMER3_EOI); 	// EOI (Read Timer3 EOI register)

        return (irq_handler_t) IRQ_HANDLED;
}



/// Module operations Vector for Acelerometer and Display
static struct file_operations fopsAcel = {
	 .owner = THIS_MODULE,
	 .read = acel_read,
	 .write = HEX5_HEX4_write
};




/**
*	This function  is the Linux module init Function 
*	@return <>0 on ERROR  (0 on OK)
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
static int __init init_acelerometer(void)
{

        int err = 0, value;

        /// allocate the mayor and minor numbers for PIO device
        if ((err = alloc_chrdev_region (&dev_no, 0, 1, DEVICE_NAME)) < 0) {
	        return err;
        }

        /// Createthe class for  device
        myACELdev_class = class_create (THIS_MODULE, DEVICE_NAME);

        /// Allocate and initialize the char device
        myACELdev_cdev = cdev_alloc ();
        myACELdev_cdev->ops = &fopsAcel;
        myACELdev_cdev->owner = THIS_MODULE;

        /// myACELdev cdev registration (on kernel)
        if ((err = cdev_add (myACELdev_cdev, dev_no, 1)) < 0) {
	        return err;
        }
        device_create (myACELdev_class, NULL, dev_no, NULL, DEVICE_NAME );
        
        // Call to generate a virtual address region for the FPGA lightweight bridge Devices
        LW_virtual = ioremap_nocache (LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

        // Call to generate a virtual address region for the HPS Timer3
        HPS_TIMER3_virtual = ioremap_nocache (HPS_TIMER3 , HPS_TIMER3_SPAN);

        // Initialize LEDR
        LEDR_ptr = (uint32_t *) (LW_virtual + LEDR_BASE);       // init virtual address for LEDR port
        *LEDR_ptr = 0x8;                                        // turn on the led

        // Initialize JP2
        JP2_ptr         = (uint32_t *) (LW_virtual + JP2_BASE); // init virtual address for JP2 port
        JP2_Data        =  JP2_ptr + 0;                         // init virtual address for JP2 Data  
        JP2_Direction	=  JP2_ptr + 1;                         // init virtual address for JP2 Direction      
        JP2_Imask       =  JP2_ptr + 2;                         // init virtual address for JP2 Interrupt Mask            
        JP2_Edge        =  JP2_ptr + 3;                         // init virtual address for JP2 Edge

        // Initialize HPS Timer2
        HPS_Timer3_ptr       = (uint32_t *) HPS_TIMER3_virtual; // init virtual address for HPS_Timer3_ptr Device Registers
        HPS_TIMER3_Load      =  HPS_Timer3_ptr + 0;             // init virtual address for HPS Timer3 Load Register
        HPS_TIMER3_Counter   =  HPS_Timer3_ptr + 1;             // init virtual address for HPS Timer3 Counter Register
        HPS_TIMER3_Control   =  HPS_Timer3_ptr + 2;             // init virtual address for HPS Timer3 Control Register
        HPS_TIMER3_EOI       =  HPS_Timer3_ptr + 3;             // init virtual address for HPS Timer3 EOI Register

	// Initialize HEX5_HEX4 display
        HEX5_HEX4_ptr = (uint32_t *) (LW_virtual + HEX5_HEX4_BASE);

        // Configure Timer3
        config_TIMER3();

        // Configure Acelerometer (JP2)
        config_ACEL();

         // Register the interrupt handler of the timer3
        value = request_irq (HPS_TIMER3_IRQ, (irq_handler_t) hps_timer3_irq_handler, IRQF_SHARED, 
        "hps_timer3_irq_handler", (void *) (hps_timer3_irq_handler));

        *LEDR_ptr = 0x200; 

        return 0;
}



/**
*	This function  is the Linux module exit Function 
///	@author A.Cabrera y J.Arin
///	@date 2020/12/13
*/
static void __exit cleanup_acelerometer(void)
{
	
        //Configure TIMER3 Control Register (Disable Timer3)
        *HPS_TIMER3_Control = 0b000;	//Interrupt mask->0,Mode->0,Enable->0

        // Turn off LEDs
        *LEDR_ptr = 0;  

        // Important !!!! Mandatory Task!!! Free all configure IRQ
        free_irq (HPS_TIMER3_IRQ, (void*) hps_timer3_irq_handler);

        device_destroy (myACELdev_class, dev_no);
        cdev_del (myACELdev_cdev);
        class_destroy (myACELdev_class);
        unregister_chrdev_region (dev_no, 1);

}


/**
*	This function  is the Linux module read Function 
*	Called when a process reads from acelerometer device, if it is ready. 
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
static ssize_t acel_read(struct file *filp, char *userbuffer, size_t length, loff_t *offset){

        size_t bytes;
        size_t ret;

        DataRW = Ton;

        printk(KERN_INFO "READ:offset=%ld   Key_Val(dec):%d  Hex_Key(0x%0x)\n", 
	        (long)*offset, DataRW, DataRW);
	        
        bytes = sizeof(DataRW);
	        
        bytes = bytes > length ? length : bytes; // too much to send at once?

        if(bytes!=0) ret=copy_to_user (userbuffer, (void*) &DataRW, bytes);

        return bytes;

}


/**
*	This function  is the Linux module write function 
*	Called when a process writes to HEX5_HEX4 (display) device
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
static ssize_t HEX5_HEX4_write(struct file *filp, const char *userbuffer, size_t length, loff_t *offset) {
	
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
		*HEX5_HEX4_ptr = Data;
		retval = bytes;
	}
	
	return retval;

}



module_init(init_acelerometer);
module_exit(cleanup_acelerometer);

