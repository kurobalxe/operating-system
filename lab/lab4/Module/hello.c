#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int mytest = 100;
module_param(mytest, int, 0644);

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello, Kernel! mytest = %d\n", mytest);
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Goodbye, Kernel!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("A simple hello kernel module");