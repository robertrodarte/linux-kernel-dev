#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// Defines the license, description, and author of the module
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple Hello World kernel module");
MODULE_AUTHOR("Robert Rodarte");

/**
 * This function is called when the module is loaded into the kernel.
 * It prints "Hello World!" to the kernel log using printk.
 */
static int __init hello_init(void)
{
    pr_info("Hello World!\n");
    return 0;
}

/**
 * This function is called when the module is removed from the kernel.
 * It prints "Goodbye World!" to the kernel log using printk.
 */
static void __exit hello_cleanup(void)
{
    pr_info("Goodbye World!\n");
}

// Define the init and cleanup functions with the kernel
module_init(hello_init);
module_exit(hello_cleanup);