#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/types.h> 
#include <linux/kdev_t.h>
#include <linux/fs.h>   
#include <linux/cdev.h> 
#include <asm/uaccess.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>

#define	GPIO_OUT	20			// GPIO_105
#define	GPIO_IN		21			// GPIO_148

static dev_t gpio_dev;      //defines structure to hold major and minor number of device//

struct cdev gpio_cdev;      //defines structure to hold character device properties//

static int gpio_lock = 0;   //flag to show when module is in use//

volatile char gpio_in_value = 0;    //defines structure to hold value from GPIO_IN-volatile as it's written by interrupt//

static irq_handler_t InterruptHandler( unsigned int irq, struct pt_regs *regs ) //defines function of the interrupt//
{		
	gpio_in_value = gpio_get_value( GPIO_IN ); //reads gpio value, places value in gpio_in_vaule//

	printk( KERN_INFO "gpio_dev: %s got GPIO_IN with value %c\n", 
                __func__, gpio_in_value+'0' );

	return (irq_handler_t) IRQ_HANDLED; 
}


int gpio_open(struct inode *inode, struct file *file)   //opens connection to module//
{
    int ret = 0;

    printk( KERN_INFO "gpio_dev: %s\n", __func__ );
    if( gpio_lock > 0 )     //checks if module is already in use//
    {
	ret = -EBUSY;      //EBUSY = error code for device locked or busy//
    }
    else
        gpio_lock++; //sets flag to busy//

   return( ret );
}

int gpio_close(struct inode *inode, struct file *file)  //closes connection to module//
{
    printk( KERN_INFO "gpio_dev: %s\n", __func__ );
    
    gpio_lock = 0;  //sets flag to not busy//

    return( 0 );
}


ssize_t gpio_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)     //reads value from device//
{
    char buffer[2];

    printk(KERN_INFO "gpio read (count=%d, offset=%d)\n", (int)count, (int)*f_pos );    

    buffer[0] = gpio_in_value;
    buffer[1] = 0;

    copy_to_user( buf, buffer, 1 );

    return 1;
}

ssize_t gpio_write(struct file *filp, const char *buffer, size_t length, loff_t * offset)   //writes value to device//
{
    int	n = 0;

    while( length )
    {
	if( *buffer == '0' )
	{
	    gpio_set_value( GPIO_OUT, 0 );
	    printk( KERN_INFO "gpio_dev: %s wrote %c to GPIO_OUT\n", __func__, *buffer );
	}
	if( *buffer == '1' )
	{
	    gpio_set_value( GPIO_OUT, 1 );
	    printk( KERN_INFO "gpio_dev: %s wrote %c to GPIO_OUT\n", __func__, *buffer );
	}
	buffer++;
	length--;
	n++;
    }

    return( n );
}


struct file_operations gpio_fops = {    //fops structure links device functions to virtual file system functions//
    .owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
    .open = gpio_open,
    .release = gpio_close,
};

static int __init gpio_module_init(void)    //Module initialisation function//
{
    char buffer[64];    //to hold messages to send to command line//
    int	ret = 0;    //return value....??????????????????????????????????????????????????????//

    printk(KERN_INFO "Loading gpio_module\n");  //Prints each time module is loaded//

    alloc_chrdev_region(&gpio_dev, 0, 1, "gpio_dev");   //allocates a major and minor number to the device//
    printk(KERN_INFO "%s\n", format_dev_t(buffer, gpio_dev));   //prints major and minor number to cmdline//

    cdev_init(&gpio_cdev, &gpio_fops);  //initialises a character device structure//
    gpio_cdev.owner = THIS_MODULE;  //
    cdev_add(&gpio_cdev, gpio_dev, 1);  //adds the cdev structure to the system//

    if( gpio_request( GPIO_OUT, "gpio_dev" ) )  //checks whether GPIO_OUT (gpio 20) is available//
    {
	printk( KERN_INFO "gpio_dev: %s unable to get GPIO_OUT\n", __func__ ); //tells user GPIO unavailable//
	ret = -EBUSY;  //what is ret and ebusy???????????????????????????????????????//
	goto Done;     //breaks from if statements to "done:" case below//
    }

    if( gpio_request( GPIO_IN, "gpio_dev" ) )    //checks whether GPIO_IN (gpio 21) is available//
    {
	printk( KERN_INFO "gpio_dev: %s unable to get GPIO_IN\n", __func__ ); 
	ret = -EBUSY;
	goto Done;
    }

    if( gpio_direction_output( GPIO_OUT, 0 ) < 0 )  //Changes GPIO_OUT to output(if not already), returns negative if it can't//
    {
	printk( KERN_INFO "gpio_dev: %s unable to set GPIO_OUT as output\n", __func__ );
	ret = -EBUSY;
	goto Done;
    }

    if( gpio_direction_input( GPIO_IN ) < 0 )   //changes GPIO_IN to input...see output//
    {
	printk( KERN_INFO "gpio_dev: %s unable to set GPIO_IN as input\n", __func__ );
	ret = -EBUSY;
	goto Done;
    }

    if( request_irq( gpio_to_irq( GPIO_IN ), (irq_handler_t) InterruptHandler,  //allocates interrupt line to GPIO_IN, invokes the interrupt...//
	 IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_dev", NULL ) < 0 )       //...handler, error gives negative return value//
    {
	printk( KERN_INFO "gpio_dev: %s unable to register gpio irq for GPIO_IN\n",
               __func__ );
		
       ret = -EBUSY;
       goto Done;
    }

Done:
    return ret;
}

static void __exit gpio_module_cleanup(void)    //module exit functon//
{
    printk(KERN_INFO "Cleaning-up gpio_dev.\n");

    gpio_free( GPIO_OUT );  //releases GPIO 20 from GPIO_OUT//
    gpio_free( GPIO_IN );   //releases GPIO 21 from GPIO_IN//

    free_irq( gpio_to_irq( GPIO_IN ), NULL );   //releases interrupt request line//

    gpio_lock = 0;  //sets module count flag back to 0 (shows not in use)//

    cdev_del(&gpio_cdev);   //removes the character device fromt he system//
    unregister_chrdev_region(gpio_dev, 1);  //frees character device major and minor numbers//
}

module_init(gpio_module_init);  //renames initialisation function "gpio_module_init"//
module_exit(gpio_module_cleanup);   //renames exit fucntion "gpio_module_cleanup"//
MODULE_AUTHOR("Your Name");     //states authors name//
MODULE_LICENSE("GPL");      //shows resource is open source//


//Useful links:
    https://www.tldp.org/LDP/lkmpg/2.6/html/lkmpg.html#AEN40
    https://www.kernel.org/doc/htmldocs/kernel-api/chrdev.html    
    http://www.zilogic.com/releases/bsp-1.5.1/doc/zdev-user-manual/_gpio.html
//