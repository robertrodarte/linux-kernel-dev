// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Expose a character device so userspace can open, read, and write to a kernel buffer");
MODULE_AUTHOR("Robert Rodarte");

#define BUFF_SIZE 1024

static ssize_t memdev_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos);
static ssize_t memdev_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos);

struct memdev {
	struct cdev cdev;
	struct class *class;
	char *buf;
	size_t bytes_written;
	dev_t dev_num;
};

static const struct file_operations memdev_fops = {
	.owner = THIS_MODULE,
	.read = memdev_read,
	.write = memdev_write,
};

static struct memdev memdev;

static int __init memdev_init(void)
{
	int success;

	pr_info("Initializing character device module\n");

	// Initialize the buffer
	memdev.buf = kmalloc(BUFF_SIZE, GFP_KERNEL);
	if (!memdev.buf)
		return -ENOMEM;

	// Grab device number
	success = alloc_chrdev_region(&memdev.dev_num, 0, 1, "memdev");
	if (success < 0) {
		pr_err("Error grabbing major number\n");
		kfree(memdev.buf);
		return success;
	}

	// Initialze and register the device
	cdev_init(&memdev.cdev, &memdev_fops);
	success = cdev_add(&memdev.cdev, memdev.dev_num, 1);
	if (success < 0) {
		// Undo alloc_chrdev_region before returning
		pr_err("Error registering character device\n");
		unregister_chrdev_region(memdev.dev_num, 1);
		kfree(memdev.buf);
		return success;
	}

	// Create the device node (dev/memdev0)
	memdev.class = class_create("memdev");
	if (IS_ERR(memdev.class)) {
		//  Undo cdev_add and alloc_chrdev_region before returning
		pr_err("Error creating device class\n");
		cdev_del(&memdev.cdev);
		unregister_chrdev_region(memdev.dev_num, 1);
		kfree(memdev.buf);
		return PTR_ERR(memdev.class);
	}
	device_create(memdev.class, NULL, memdev.dev_num, NULL, "memdev0");

	// Module loaded
	pr_info("Character device module loaded\n");
	return 0;
}

static void __exit memdev_cleanup(void)
{
	pr_info("Unloading character device module\n");

	// Destroy the device
	device_destroy(memdev.class, memdev.dev_num);

	// Destroy the class
	class_destroy(memdev.class);

	// Unregister the device
	cdev_del(&memdev.cdev);

	// Free device number
	unregister_chrdev_region(memdev.dev_num, 1);

	// Free the buffer
	kfree(memdev.buf);

	pr_info("Character device module unloaded\n");
}

static ssize_t memdev_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos)
{
	// Ensure the write is within the bounds of the allocated buffer
	if (*ppos + count > BUFF_SIZE) {
		pr_err("Write is out of bounds\n");
		return -EFAULT;
	}

	// Copy data from user space to kernel space
	if (copy_from_user(memdev.buf + *ppos, ubuf, count)) {
		pr_err("Failed to copy data from user space\n");
		return -EFAULT;
	}

	// Update the file position, bytes written, and reset
	*ppos += count;
	memdev.bytes_written = *ppos;
	*ppos = 0;

	return count;
}

static ssize_t memdev_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos)
{
	// Check for end of file
	if (*ppos >= memdev.bytes_written)
		return 0;

	// Ensure the read is within the bounds of the allocated buffer
	if (count > memdev.bytes_written - *ppos)
		count = memdev.bytes_written - *ppos;

	// Copy data from kernel space to user space
	if (copy_to_user(ubuf, memdev.buf + *ppos, count)) {
		pr_err("Failed to copy data to user space\n");
		return -EFAULT;
	}

	// Update the file position
	*ppos += count;

	return count;
}

module_init(memdev_init);
module_exit(memdev_cleanup);
