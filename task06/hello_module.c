#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <asm/string.h>

MODULE_AUTHOR("Vladimir Petrigo");
MODULE_LICENSE("Dual BSD/GPL");

#define FIRSTMINOR 0
#define DEV_COUNT 1
#define DEV_NAME "eudyptula"

int hello_open(struct inode *, struct file *);
ssize_t hello_read(struct file *, char __user *, size_t, loff_t *);
ssize_t hello_write(struct file *, const char __user *, size_t, loff_t *);

struct hello_chrdev {
	struct cdev hello_cdev;
	dev_t devno;
};

struct file_operations hello_chrdev_ops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.read = hello_read,
	.write = hello_write
};

struct hello_chrdev hello_dev;

int hello_open(struct inode *inode, struct file *filp)
{
	filp->private_data = container_of(inode->i_cdev, 
					struct hello_chrdev, hello_cdev);

	return 0;
}

ssize_t hello_read(struct file *filp, char __user *to, size_t from, loff_t *pos)
{
	char buf[32] = {'\0'};
	struct hello_chrdev *phello = filp->private_data;
	ssize_t result = 0;
	
	result = snprintf(buf, sizeof(buf), "%d %d", MAJOR(phello->devno), 
					MINOR(phello->devno));

	if (result < 0)
		goto fail;	

	if (copy_to_user(to, buf, result)) {
		result = -EFAULT;
		goto fail;
	}	

fail:
	return result;
}

ssize_t hello_write(struct file *filp, const char __user *from, size_t to, loff_t *pos)
{
	char buf[32] = {'\0'};
	struct hello_chrdev *phello = filp->private_data;
	size_t devno_len = 0;
	size_t user_len = 0;
	int result = -EINVAL;

	pr_debug("hello: User input %s\n", from);
	snprintf(buf, sizeof(buf), "%d %d", MAJOR(phello->devno),
					MINOR(phello->devno));	
	devno_len = strlen(buf);
	user_len = strlen(from);
	result = memcmp(from, buf, 
			(devno_len > user_len) ? user_len : devno_len);

	if (result != 0)
		goto fail;

	result = devno_len;
fail:
	return result;
}

int __init hello_init(void)
{
	int result = alloc_chrdev_region(&hello_dev.devno, FIRSTMINOR, 
					DEV_COUNT, DEV_NAME);

	if (result < 0) {
		goto fail;
	}

	cdev_init(&hello_dev.hello_cdev, &hello_chrdev_ops);
	result = cdev_add(&hello_dev.hello_cdev, hello_dev.devno, DEV_COUNT);

	if (result < 0) {
		goto fail;
	}

	pr_debug("hello: Hello World!\n");

	return 0;

fail:
	return result;
}

void __exit hello_exit(void)
{
	unregister_chrdev_region(hello_dev.devno, DEV_COUNT);
	cdev_del(&hello_dev.hello_cdev);
	pr_debug("hello: Goodbye!\n");
}

module_init(hello_init);
module_exit(hello_exit);
