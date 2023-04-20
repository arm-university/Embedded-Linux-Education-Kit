#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>   
#include <linux/cdev.h> 


static dev_t hello_dev;
struct cdev hello_cdev;
static char buffer[64];

struct file_operations hello_fops = {
    .owner = THIS_MODULE,
 
};

static int __init hello_module_init(void)
{
    printk(KERN_INFO "Loading HelloWorld_module.\n");

    alloc_chrdev_region(&hello_dev, 0, 1, "hello_dev");
    printk(KERN_INFO "%s\n", format_dev_t(buffer, hello_dev));

    cdev_init(&hello_cdev, &hello_fops);
    hello_cdev.owner = THIS_MODULE;
    cdev_add(&hello_cdev, hello_dev, 1);

    return 0;
}

static void __exit hello_module_cleanup(void)
{
    printk(KERN_INFO "Cleaning-up hello_dev.\n");
    cdev_del(&hello_cdev);
    unregister_chrdev_region(hello_dev, 1);
}

module_init(hello_module_init);
module_exit(hello_module_cleanup);

MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");

