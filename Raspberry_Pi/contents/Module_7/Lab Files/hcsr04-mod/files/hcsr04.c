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

#include <linux/kobject.h> 
#include <linux/sysfs.h>

#include <linux/time.h>
#include <linux/ktime.h>
#include <asm/delay.h> 
#include <linux/delay.h>

#define	GPIO_OUT	20			// GPIO20
#define	GPIO_IN		21			// GPIO21

static dev_t hcsr04_dev;		//defines structure to hold major and minor number of device//

struct cdev hcsr04_cdev;		 //defines structure to hold character device properties//

static int hcsr04_lock = 0;		//flag to show when module is in use//

static struct kobject *hcsr04_kobject;

static ktime_t rising, falling;

int hcsr04_open(struct inode *inode, struct file *file)		//opens module connecton, asserts "busy"//
{
    int ret = 0;

    printk( KERN_INFO "hcsr04_dev: %s\n", __func__ );
    if( hcsr04_lock > 0 )
    {
	ret = -EBUSY;
    }
    else
        hcsr04_lock++;

   return( ret );
}

int hcsr04_close(struct inode *inode, struct file *file)	//cloeses connection, sets state to not busy//
{
    printk( KERN_INFO "hcsr04_dev: %s\n", __func__ );

    hcsr04_lock = 0;

    return( 0 );
}

ssize_t hcsr04_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int ret;
    int pulse;

    printk( KERN_INFO "hcsr04_dev: %s\n", __func__ );

    pulse = (int)ktime_to_us( ktime_sub( falling, rising ) );

    ret = copy_to_user( buf, &pulse, 4 );

    return 4;
}

ssize_t hcsr04_write(struct file *filp, const char *buffer, size_t length, loff_t * offset)
{
    printk( KERN_INFO "hcsr04_dev: %s\n", __func__ );

    gpio_set_value( GPIO_OUT, 0 );
    gpio_set_value( GPIO_OUT, 1 );
    udelay( 10 );
    gpio_set_value( GPIO_OUT, 0 );

    while( gpio_get_value( GPIO_IN ) == 0 )
	;
    rising = ktime_get();

    while( gpio_get_value( GPIO_IN ) == 1 )
	;
    falling = ktime_get();

    return( 1 );
}


struct file_operations hcsr04_fops = {
    .owner = THIS_MODULE,
    .read = hcsr04_read,
    .write = hcsr04_write,
    .open = hcsr04_open,
    .release = hcsr04_close,
};

static ssize_t hcsr04_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{
    printk( KERN_INFO "hcsr04_dev: %s\n", __func__ );
    
    return sprintf(buf, "%d\n", ktime_to_us(ktime_sub(falling,rising)));
}

static ssize_t hcsr04_store(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf, size_t count)
{
    printk( KERN_INFO "hcsr04_dev: %s\n", __func__ );

    return 1;
}

static struct kobj_attribute hcsr04_attribute =__ATTR(hcsr04, 0660, hcsr04_show, hcsr04_store);

static int __init hcsr04_module_init(void)
{
    char buffer[64];
    int	ret = 0;

    printk(KERN_INFO "Loading hcsr04_module\n");

    alloc_chrdev_region(&hcsr04_dev, 0, 1, "hcsr04_dev");
    printk(KERN_INFO "%s\n", format_dev_t(buffer, hcsr04_dev));

    cdev_init(&hcsr04_cdev, &hcsr04_fops);
    hcsr04_cdev.owner = THIS_MODULE;
    cdev_add(&hcsr04_cdev, hcsr04_dev, 1);


    if( gpio_request( GPIO_OUT, "hcsr04_dev" ) )
    {
        printk( KERN_INFO "hcsr04_dev: %s unable to get GPIO_OUT\n", __func__ );
        ret = -EBUSY;
	goto Done;
    }

    if( gpio_request( GPIO_IN, "hcsr04_dev" ) )
    {
	printk( KERN_INFO "hcsr04_dev: %s unable to get GPIO_IN\n", __func__ );
	ret = -EBUSY;
	goto Done;
    }

    if( gpio_direction_output( GPIO_OUT, 0 ) < 0 )
    {
        printk( KERN_INFO "hcsr04_dev: %s unable to set GPIO_OUT as output\n", __func__ );
	ret = -EBUSY;
	goto Done;
    }

    if( gpio_direction_input( GPIO_IN ) < 0 )
    {
        printk( KERN_INFO "hcsr04_dev: %s unable to set GPIO_IN as input\n", __func__ );
        ret = -EBUSY;
        goto Done;
    }

    hcsr04_kobject = kobject_create_and_add("hcsr04", kernel_kobj);
    if(!hcsr04_kobject)
    {
	ret = -ENOMEM;
	goto Done;
    }
    ret = sysfs_create_file(hcsr04_kobject, &hcsr04_attribute.attr);
    if( ret ) 
    {
	printk( KERN_INFO "failed to create the foo file in /sys/kernel/hcsr04\n");
	ret = -ENOMEM;
	goto Done;
    }


Done:
    return ret;
}

static void __exit hcsr04_module_cleanup(void)
{
    printk(KERN_INFO "Cleaning-up hcsr04_dev.\n");

    gpio_free( GPIO_OUT );
    gpio_free( GPIO_IN );

    hcsr04_lock = 0;

    cdev_del(&hcsr04_cdev);
    unregister_chrdev_region( hcsr04_dev, 1 );

    kobject_put( hcsr04_kobject );
}

module_init(hcsr04_module_init);
module_exit(hcsr04_module_cleanup);

MODULE_AUTHOR("Your name");
MODULE_LICENSE("GPL");
