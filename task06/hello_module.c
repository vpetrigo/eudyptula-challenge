#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <asm/string.h>

MODULE_AUTHOR("Vladimir Petrigo");
MODULE_LICENSE("Dual BSD/GPL");

#define FIRSTMINOR 0
#define DEV_COUNT 1
#define DEV_NAME "eudyptula"
#define BUF_SIZE 32

static int hello_open(struct inode *, struct file *);
static ssize_t hello_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t hello_write(struct file *, const char __user *, size_t, loff_t *);

static const struct file_operations hello_misc_ops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.read = hello_read,
	.write = hello_write
};

static struct miscdevice hello_mdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEV_NAME,
	.fops = &hello_misc_ops
};

static int hello_open(struct inode *inode, struct file *filp)
{
	pr_debug("hello_module: hello open\n");
	filp->private_data = &inode->i_rdev;
	pr_debug("hello_module: open miscdevice with minor ID %d\n", MINOR(inode->i_rdev));

	return 0;
}

static ssize_t hello_read(struct file *filp, char __user *to, size_t length, loff_t *pos)
{
	char buf[BUF_SIZE] = {'\0'};
	const dev_t *devp = filp->private_data;
	ssize_t result = 0;
	
	result = snprintf(buf, sizeof(buf), "%d\n", MINOR(*devp));

	if (result < 0)
		goto fail;	

	if (copy_to_user(to, buf, result)) {
		result = -EFAULT;
		goto fail;
	}	

fail:
	return result;
}

static ssize_t hello_write(struct file *filp, const char __user *from, size_t length, loff_t *pos)
{
	char buf[BUF_SIZE] = {'\0'};
	const dev_t *devp = filp->private_data;	
	int result = -EINVAL;
	size_t devno_len = snprintf(buf, sizeof(buf), "%d", MINOR(*devp));	

	pr_debug("hello: User input %s\n", from);
	result = memcmp(from, buf, 
			(devno_len > length) ? length : devno_len);

	if (result != 0)
	{
		result = -EINVAL;
		goto fail;
	}

	result = devno_len;
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
