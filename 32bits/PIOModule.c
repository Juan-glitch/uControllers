///
///	@file PIOModule.c 
///	@brief Este fichero C contiene un módulo de kernel Linux
///	       que gestiona los LEDs y los SWs y ofrece dos funciones de read y write.
///                READ: SWs
///                WRITE: LEDs
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
MODULE_AUTHOR("A.Cabrera y J.Arin");
MODULE_DESCRIPTION("Read/write KEY-LEDS-SW driver");

#define KEYS_DataReg	 	0
#define KEYS_DirectionReg 	1
#define KEYS_IRQEnable 		2
#define KEYS_EdgeCapReg 	3

/// Proptotipe switch_read function 
static ssize_t switch_read(struct file *, char *, size_t, loff_t *);


/// Proptotipe leds_write function
static ssize_t leds_write(struct file *, const char *, size_t , loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "myPIO"

static dev_t dev_no = 0;
static struct cdev *myPIOdev_cdev = NULL;
static struct class *myPIOdev_class = NULL;

static uint32_t DataRW;

void *LW_virtual; ///  Virtual Address global Vars			

volatile int * LEDR_ptr;  /// virtual address pointer to red LEDGs	
		
volatile int * SWITCH_ptr; /// virtual address for the SW port  


/*
///	This function  will be used to generate the virtual address variables
///	@author G. Alvarez
///	@date 11/23/2018
*/
void virtualAddressInitialize(void)
{
	// generate a virtual address for the FPGA lightweight bridge
	LW_virtual = ioremap_nocache (LW_BRIDGE_BASE, LW_BRIDGE_SPAN);

	// Set virtual address pointer to I/O port
	LEDR_ptr = (unsigned int *) (LW_virtual + LEDR_BASE);

	// Set virtual address pointer to I/O port
	SWITCH_ptr = (unsigned int *) (LW_virtual + SW_BASE);

}


///*
///	This function  will be used to remove the virtual address variable
///	@author G. Alvarez
///	@date 11/23/2018
///
void virtualAddressRemove(void)
{
	// Unmap I/O memory from kernel address space
	iounmap(LW_virtual);	 
	
}



/**
*	This function  will be used to initialize the LEDR port
*	@author G. Alvarez
*	@date 11/23/2018
*/
void initializeLeds(void)
{
	
	*LEDR_ptr = 0x1;    //  (LEDR)

}

/**
*	This function  will be used to initialize the SW port
*	@author G. Alvarez
*	@date 11/23/2018
*/
void initializeSwitch(void)
{
	
	// Nothing to do
	;

}


/**
*	This function  will be used to unconfigure the SW port
*	@author G. Alvarez
*	@date 11/23/2018
*/
void unconfigureSwitch(void)
{
	// Nothing to do
	;
	
	
}


/**
*	This function  will be used to unconfigure the LEDR port
*	@author G. Alvarez
*	@date 11/23/2018
*/
void unconfigureLeds(void)
{
	// Turn OFF all leds
	*LEDR_ptr = 0;
	
}

		
/**
*	This function  will be used to initialize the myPIO Device 
*	@author G. Alvarez
*	@date 11/23/2018
*/
void intHwDevices(void) {
	
	virtualAddressInitialize();
	
	initializeLeds();
	
	initializeSwitch();

}


/// Module operations Vector for Keys_Leds
static struct file_operations fopsKeys = {
	 .owner = THIS_MODULE,
	 .read = switch_read,
	 .write = leds_write
};

	
/**
*	This function  is the Linux module init Function 
*	@return <>0 on ERROR  (0 on OK)
*	@author G. Alvarez
*	@date 11/23/2018
*/
static int __init init_PIOdev(void){
	int err = 0;
//	uint8_t devid;
	
	/// allocate the mayor and minor numbers for PIO device
	if ((err = alloc_chrdev_region (&dev_no, 0, 1, DEVICE_NAME)) < 0) {
		return err;
	}
	
	/// Createthe class for  device
	myPIOdev_class = class_create (THIS_MODULE, DEVICE_NAME);

	/// Allocate and initialize the char device
	myPIOdev_cdev = cdev_alloc ();
	myPIOdev_cdev->ops = &fopsKeys;
	myPIOdev_cdev->owner = THIS_MODULE;

	/// myPIOdev cdev registration (on kernel)
	if ((err = cdev_add (myPIOdev_cdev, dev_no, 1)) < 0) {
		return err;
	}
	device_create (myPIOdev_class, NULL, dev_no, NULL, DEVICE_NAME );

	//Init Devices....Leds, Keys,..
	intHwDevices();	
	
        printk(KERN_INFO "START %s Device\n", DEVICE_NAME);
	
	return 0;
}


	
/**
*	This function  is the Linux module exit Function 
*	@author G. Alvarez
*	@date 11/23/2018
*/
static void __exit stop_PIOdev(void){
	
	
	device_destroy (myPIOdev_class, dev_no);
	cdev_del (myPIOdev_cdev);
	class_destroy (myPIOdev_class);
	unregister_chrdev_region (dev_no, 1);

	unconfigureLeds();
	unconfigureSwitch();
	
	virtualAddressRemove();

	 
}



/**
*	This function  is the Linux module read Function 
*	Called when a process reads from SWITCH device, if it is ready. 
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
static ssize_t switch_read(struct file *filp, char *userbuffer, size_t length, loff_t *offset){

	size_t bytes;
 	size_t ret;
	
	DataRW = *(SWITCH_ptr);
	
        printk(KERN_INFO "READ:offset=%ld   Key_Val(dec):%d  Hex_Key(0x%0x)\n", (long)*offset, DataRW, DataRW);
		
	bytes = sizeof(DataRW);
		
	bytes = bytes > length ? length : bytes; // too much to send at once?
	
	if(bytes!=0) ret=copy_to_user (userbuffer, (void*) &DataRW, bytes);
	
	return bytes;

}



/**
*	This function  is the Linux module write function 
*	Called when a process writes to LEDS device
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
static ssize_t leds_write(struct file *filp, const char *userbuffer, size_t length, loff_t *offset) {
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
		*LEDR_ptr = Data;
		retval = bytes;
	}
	
	return retval;

}

	

MODULE_LICENSE("GPL");
module_init (init_PIOdev);
module_exit (stop_PIOdev);
