#include <linux/debugfs.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/stat.h>

MODULE_AUTHOR("Vladimir Petrigo");
MODULE_DESCRIPTION("Hello module with Debugfs");
MODULE_LICENSE("Dual MIT/GPL");

#define HELLO_DEBUGFS_DIR "eudyptula"
#define HELLO_DEBUGFS_FILE "id"
#define HELLO_DEBUGFS_JIFFIES_FILE "jiffies"

static const char assigned_id[] = "100f321669eb";
#define ASSIGNED_ID_SIZE (sizeof assigned_id)

struct hello_debugfs {
	struct dentry *hello_dir;
	struct dentry *id;
	struct dentry *jiffies;
};

static struct hello_debugfs hello;

ssize_t hello_read(struct file *, char __user *, size_t, loff_t *);
ssize_t hello_write(struct file *, const char __user *, size_t, loff_t *);
int hello_open(struct inode *, struct file *);
int hello_release(struct inode *, struct file *);

ssize_t jiffies_read(struct file *, char __user *, size_t, loff_t *);

static struct file_operations hello_ops = {
    .owner = THIS_MODULE, .read = hello_read, .write = hello_write};

static struct file_operations jiffies_ops = {.owner = THIS_MODULE,
					     .read = jiffies_read};

ssize_t hello_read(struct file *filp, char __user *to, size_t length,
		   loff_t *pos)
{
	return simple_read_from_buffer(to, length, pos, assigned_id,
				       ASSIGNED_ID_SIZE);
}

ssize_t hello_write(struct file *filp, const char __user *from, size_t length,
		    loff_t *pos)
{
	char buf[ASSIGNED_ID_SIZE];
	ssize_t result = -EINVAL;

	if (length != ASSIGNED_ID_SIZE - 1) {
		pr_debug("%s: Read from buffer\n", __func__);
		result = simple_write_to_buffer(buf, ASSIGNED_ID_SIZE, pos,
						from, length);
	}

	if (result != -EINVAL &&
	    strncmp(buf, assigned_id, ASSIGNED_ID_SIZE) == 0) {
		result = ASSIGNED_ID_SIZE;
	}

	return result;
}

ssize_t jiffies_read(struct file *filp, char __user *to, size_t length,
		     loff_t *pos)
{
	char buf[24];
	int ret = snprintf(buf, sizeof buf, "%lu", jiffies);

	return simple_read_from_buffer(to, length, pos, buf, ret);
}

int hello_init(void)
{
	hello.hello_dir = debugfs_create_dir(HELLO_DEBUGFS_DIR, NULL);

	if (hello.hello_dir == NULL) {
		pr_err("%s: Unable to create %s\n", __func__,
		       HELLO_DEBUGFS_DIR);
		return -EFAULT;
	}

	hello.id = debugfs_create_file(HELLO_DEBUGFS_FILE, S_IRUGO | S_IWUGO,
				       hello.hello_dir, NULL, &hello_ops);

	if (hello.id == NULL) {
		pr_err("%s: Unable to create %s\n", __func__,
		       HELLO_DEBUGFS_FILE);
		return -EFAULT;
	}

	hello.jiffies =
	    debugfs_create_file(HELLO_DEBUGFS_JIFFIES_FILE, S_IRUGO,
				hello.hello_dir, NULL, &jiffies_ops);

	if (hello.jiffies == NULL) {
		pr_err("%s: Unable to create %s\n", __func__,
		       HELLO_DEBUGFS_JIFFIES_FILE);
		return -EFAULT;
	}

	return 0;
}

void hello_exit(void)
{
	debugfs_remove(hello.id);
	debugfs_remove(hello.jiffies);
	debugfs_remove(hello.hello_dir);
	pr_debug("%s: Goodbye\n", __func__);
}

module_init(hello_init);
module_exit(hello_exit);
