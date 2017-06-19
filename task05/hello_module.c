#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/usb.h>
#include <linux/hid.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Vladimir Petrigo");

static int hello_probe(struct usb_interface *interface, 
		const struct usb_device_id *id);

static void hello_disconnect(struct usb_interface *intf);

static struct usb_device_id hello_table[] = {
	{USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
			USB_INTERFACE_PROTOCOL_KEYBOARD)},
	{}
};
MODULE_DEVICE_TABLE(usb, hello_table);

static struct usb_driver hello_driver = {
	.name = "hello",
	.probe = hello_probe,
	.disconnect = hello_disconnect,
	.id_table = hello_table
};

static int __init hello_init(void)
{
	int result = usb_register(&hello_driver);

	if (result < 0)
		pr_err("Can not register hello_driver");	

	pr_info("Hello World!\n");
	return result;
}


static void __exit hello_exit(void)
{	
	pr_info("Goodbye!\n");
	usb_deregister(&hello_driver);
}

static int hello_probe(struct usb_interface *interface, 
		const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(interface);
	
	pr_info("Hello Keyboard!\n");
	pr_info("Keyboard ID %d, path %s\n", dev->devnum, dev->devpath);
	pr_info("Manuf %s, serial %s, product %s\n", dev->manufacturer,
							dev->serial,
							dev->product);

	return 0;
}

static void hello_disconnect(struct usb_interface *intf)
{
	pr_info("Goodbye Keyboard!\n");
}

module_init(hello_init);
module_exit(hello_exit);
