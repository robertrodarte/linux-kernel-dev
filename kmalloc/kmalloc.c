#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

// Defines the license, description, and author of the module
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple kmalloc kernel module");
MODULE_AUTHOR("Robert Rodarte");

// Size of memory to allocate with kmalloc()
#define KMALLOC_SIZE 1024

// Pointer to hold the allocated memory from kmalloc()
static void *allocated;

/**
 * Initialization function for the kmalloc module.
 * Attempts to allocate memory using kmalloc() and logs the result.
 * Returns 0 on success or a negative error code on failure.
 */
static int __init kmalloc_init(void)
{
    pr_info("kmalloc module initializing\n");

    // Attempt to allocate KMALLOC_SIZE bytes of memory
    allocated = kmalloc(KMALLOC_SIZE, GFP_KERNEL);

    // Check if kmalloc() succeeded
    if (!allocated)
    {
        pr_err("kmalloc failed to allocate memory\n");
        return -ENOMEM;
    }

    pr_info("kmalloc module loaded and %d bytes allocated\n", KMALLOC_SIZE);
    return 0;
}

/**
 * Cleanup function for the kmalloc module.
 * Frees the allocated memory and logs the exit of the module.
 */
static void __exit kmalloc_exit(void)
{
    pr_info("kmalloc module exiting\n");
    kfree(allocated);
    pr_info("kmalloc module unloaded and memory freed\n");
}

// Define init and exit functions for the module
module_init(kmalloc_init);
module_exit(kmalloc_exit);