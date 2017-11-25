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
#define ASSIGNED_ID_LEN (sizeof assigned_id)

static ssize_t hello_read(struct file *filp, char __user *to, size_t length,
			  loff_t *pos)
{
	return simple_read_from_buffer(to, length, pos, assigned_id,
				       ASSIGNED_ID_LEN);
}

static ssize_t hello_write(struct file *filp, const char __user *from,
			   size_t length, loff_t *pos)
{
	char buf[ASSIGNED_ID_LEN] = {'\0'};
	ssize_t result = -EINVAL;

	if (length == ASSIGNED_ID_LEN - 1) {
		pr_debug("%s: read user buffer\n", __func__);
		result = simple_write_to_buffer(buf, ASSIGNED_ID_LEN, pos, from,
						length);
	}

	if (result != -EINVAL &&
	    strncmp(buf, assigned_id, ASSIGNED_ID_LEN) == 0) {
		result = ASSIGNED_ID_LEN;
	}

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
