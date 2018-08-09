#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/stat.h>
#include <linux/sysfs.h>

MODULE_AUTHOR("Vladimir Petrigo");
MODULE_DESCRIPTION("Hello module with sysfs");
MODULE_LICENSE("Dual MIT/GPL");

#define HELLO_SYSFS_DIR "eudyptula"
#define HELLO_SYSFS_FILE "id"
#define HELLO_SYSFS_JIFFIES_FILE "jiffies"
#define HELLO_SYSFS_FOO "foo"

static const char assigned_id[] = "100f321669eb";
#define ASSIGNED_ID_SIZE ((sizeof assigned_id) - 1)

struct foo {
	char data[PAGE_SIZE];
	rwlock_t lock;
};

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf);
static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count);

static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr,
		       char *buf);
static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t count);

static ssize_t jiffies_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf);

static struct foo foo;

static struct kobj_attribute foo_attribute = __ATTR_RW(foo);
static struct kobj_attribute id_attribute = __ATTR_RW(id);
static struct kobj_attribute jiffies_attribute = __ATTR_RO(jiffies);

static struct attribute *attrs[] = {
    &foo_attribute.attr, &id_attribute.attr, &jiffies_attribute.attr,
    NULL /* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {.attrs = attrs};

static struct kobject *hello_kobj;

static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr,
		       char *buf)
{
	return sprintf(buf, "%s\n", assigned_id);
}

static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	ssize_t result = -EINVAL;

	if (count == ASSIGNED_ID_SIZE &&
	    strncmp(buf, assigned_id, ASSIGNED_ID_SIZE) == 0) {
		result = ASSIGNED_ID_SIZE;
	}

	return result;
}

static ssize_t jiffies_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf)
{
	return sprintf(buf, "%lu\n", jiffies);
}

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	ssize_t result;

	read_lock(&foo.lock);
	result = sprintf(buf, "%s", foo.data);
	read_unlock(&foo.lock);

	return result;
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	ssize_t result;

	write_lock(&foo.lock);
	memset(foo.data, '\0', sizeof foo.data);
	result = snprintf(foo.data, sizeof foo.data, "%s", buf);
	write_unlock(&foo.lock);

	return result;
}

static int __init hello_init(void)
{
	int retval;

	hello_kobj = kobject_create_and_add(HELLO_SYSFS_DIR, kernel_kobj);

	if (!hello_kobj) {
		pr_err("%s: Unable to create hello_kobj\n", __func__);
		return -ENOMEM;
	}

	rwlock_init(&foo.lock);
	retval = sysfs_create_group(hello_kobj, &attr_group);

	if (retval) {
		pr_err("%s: Unable to create sysfs group\n", __func__);
		kobject_put(hello_kobj);
	}

	return retval;
}

static void __exit hello_exit(void)
{
	kobject_put(hello_kobj);
	pr_debug("%s: Goodbye\n", __func__);
}

module_init(hello_init);
module_exit(hello_exit);
