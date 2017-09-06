#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>

int init_module(void)
{
	pr_debug("Hello World!\n");
	return 0;
}

void cleanup_module(void)
{
	pr_debug("Goodbye!\n");
}
