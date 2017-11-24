#include <asm/string.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Vladimir Petrigo");
MODULE_LICENSE("Dual BSD/GPL");

#define FIRSTMINOR 0
#define DEV_COUNT 1
#define DEV_NAME "eudyptula"
#define BUF_SIZE 32

static ssize_t hello_write(struct file *filp, const char __user *from,
			   size_t length, loff_t *pos);
static ssize_t hello_read(struct file *filp, char __user *to, size_t length,
			  loff_t *pos);

static const struct file_operations hello_misc_ops = {
    .owner = THIS_MODULE, .read = hello_read, .write = hello_write};

static struct miscdevice hello_mdev = {
    .minor = MISC_DYNAMIC_MINOR, .name = DEV_NAME, .fops = &hello_misc_ops};

static const char assigned_id[] = "100f321669eb";

static ssize_t hello_read(struct file *filp, char __user *to, size_t length,
			  loff_t *pos)
{
	ssize_t result = sizeof assigned_id;
	if (copy_to_user(to, assigned_id, sizeof assigned_id)) {
		result = -EFAULT;
		goto fail;
	}

fail:
	return result;
}

static ssize_t hello_write(struct file *filp, const char __user *from,
			   size_t length, loff_t *pos)
{
	char buf[sizeof assigned_id] = {'\0'};
	ssize_t result;

	if (length != (sizeof assigned_id) - 1 ||
	    copy_from_user(buf, from, length) ||
	    strncmp(assigned_id, buf, sizeof assigned_id)) {
		result = -EINVAL;
		goto fail;
	}

	result = sizeof assigned_id;
fail:
	return result;
}

static int hello_init(void)
{
	int retval = misc_register(&hello_mdev);

	if (retval < 0)
		goto fail;

	pr_debug("hello: Hello World!\n");

	return 0;

fail:
	return retval;
}

static void hello_exit(void)
{
	misc_deregister(&hello_mdev);
	pr_debug("hello: Goodbye!\n");
}

module_init(hello_init);
module_exit(hello_exit);
