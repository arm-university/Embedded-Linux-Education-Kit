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

static dev_t gpio_dev;

struct cdev gpio_cdev;

static int gpio_lock = 0;

volatile char gpio_in_value = 0;

static irq_handler_t InterruptHandler( unsigned int irq, struct pt_regs *regs )
{		
	gpio_in_value = gpio_get_value( GPIO_IN );

	printk( KERN_INFO "gpio_dev: %s got GPIO_IN with value %c\n", 
                __func__, gpio_in_value+'0' );

	return (irq_handler_t) IRQ_HANDLED; 
}


int gpio_open(struct inode *inode, struct file *file)
{
    int ret = 0;

    printk( KERN_INFO "gpio_dev: %s\n", __func__ );
    if( gpio_lock > 0 )
    {
	ret = -EBUSY;
    }
    else
        gpio_lock++;

   return( ret );
}

int gpio_close(struct inode *inode, struct file *file)
{
    printk( KERN_INFO "gpio_dev: %s\n", __func__ );
    
    gpio_lock = 0;

    return( 0 );
}


ssize_t gpio_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    char buffer[2];

    printk(KERN_INFO "gpio read (count=%d, offset=%d)\n", (int)count, (int)*f_pos );

    buffer[0] = gpio_in_value;
    buffer[1] = 0;

    copy_to_user( buf, buffer, 1 );

    return 1;
}

ssize_t gpio_write(struct file *filp, const char *buffer, size_t length, loff_t * offset)
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


struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .read = gpio_read,
    .write = gpio_write,
    .open = gpio_open,
    .release = gpio_close,
};

static int __init gpio_module_init(void)
{
    char buffer[64];
    int	ret = 0;

    printk(KERN_INFO "Loading gpio_module\n");

    alloc_chrdev_region(&gpio_dev, 0, 1, "gpio_dev");
    printk(KERN_INFO "%s\n", format_dev_t(buffer, gpio_dev));

    cdev_init(&gpio_cdev, &gpio_fops);
    gpio_cdev.owner = THIS_MODULE;
    cdev_add(&gpio_cdev, gpio_dev, 1);

    if( gpio_request( GPIO_OUT, "gpio_dev" ) )
    {
	printk( KERN_INFO "gpio_dev: %s unable to get GPIO_OUT\n", __func__ );
	ret = -EBUSY;
	goto Done;
    }

    if( gpio_request( GPIO_IN, "gpio_dev" ) )
    {
	printk( KERN_INFO "gpio_dev: %s unable to get GPIO_IN\n", __func__ );
	ret = -EBUSY;
	goto Done;
    }

    if( gpio_direction_output( GPIO_OUT, 0 ) < 0 )
    {
	printk( KERN_INFO "gpio_dev: %s unable to set GPIO_OUT as output\n", __func__ );
	ret = -EBUSY;
	goto Done;
    }

    if( gpio_direction_input( GPIO_IN ) < 0 )
    {
	printk( KERN_INFO "gpio_dev: %s unable to set GPIO_IN as input\n", __func__ );
	ret = -EBUSY;
	goto Done;
    }

    if( request_irq( gpio_to_irq( GPIO_IN ), (irq_handler_t) InterruptHandler, 
	 IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_dev", NULL ) < 0 )
    {
	printk( KERN_INFO "gpio_dev: %s unable to register gpio irq for GPIO_IN\n",
               __func__ );
		
       ret = -EBUSY;
       goto Done;
    }

Done:
    return ret;
}

static void __exit gpio_module_cleanup(void)
{
    printk(KERN_INFO "Cleaning-up gpio_dev.\n");

    gpio_free( GPIO_OUT );
    gpio_free( GPIO_IN );

    free_irq( gpio_to_irq( GPIO_IN ), NULL );

    gpio_lock = 0;

    cdev_del(&gpio_cdev);
    unregister_chrdev_region(gpio_dev, 1);
}

module_init(gpio_module_init);
module_exit(gpio_module_cleanup);

MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
