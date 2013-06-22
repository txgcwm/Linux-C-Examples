#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>



#define DNWBULK_MAJOR	102
#define DNWBULK_MINOR	0
#define BULKOUT_BUFFER_SIZE	1024
#define DRIVER_NAME		"dnwbulk"

struct dnwbulk_dev
{
	struct 	usb_device *udev;
	struct 	mutex io_mutex;
	char	*bulkout_buffer;
	__u8	bulk_out_endpointAddr;
};

static struct usb_class_driver dnwbulk_class;
static struct usb_driver dnwbulk_driver;

static struct usb_device_id dnwbulk_table[]= {
	{USB_DEVICE(0x067b, 0x2303)},
	{ }
};

static void dnwbulk_disconnect(struct usb_interface *interface)
{
	struct dnwbulk_dev *dev = NULL;

	printk(KERN_INFO "dnwbulk:dnwbulk disconnected!\n");
	dev = usb_get_intfdata(interface);
	if( NULL != dev )
		kfree(dev);
	usb_deregister_dev(interface, &dnwbulk_class);

	return;
}

static ssize_t dnwbulk_read(struct file *file, char __user *buf, size_t len, loff_t *loff)
{
	return -EPERM;
}

static ssize_t dnwbulk_write(struct file *file, const char __user *buf, size_t len, loff_t *loff)
{	
	int ret;
	int actual_length;
	size_t to_write;
	size_t total_writed;
	size_t block = BULKOUT_BUFFER_SIZE;
	struct dnwbulk_dev *dev = file->private_data;
	
	total_writed = 0;
	while(len > 0)
	{
		to_write = min(len, block);
	
		if(copy_from_user(dev->bulkout_buffer, buf+total_writed, to_write))
		{
			printk(KERN_ERR "dnwbulk:copy_from_user failed!\n");
			return -EFAULT;	
		}	
		
		ret = usb_bulk_msg(dev->udev, usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
				dev->bulkout_buffer,
				to_write,
				&actual_length,
				HZ*3);
		while(ret == -ETIMEDOUT)
		{
			printk(KERN_ERR "I really can't believe\n");
			printk(KERN_ERR "dnwbulk:usb_bulk_msg ret %d!\n",ret);
		}
		if(ret || (actual_length != to_write))
		{	
			printk(KERN_ERR "dnwbulk:usb_bulk_msg failed!\n");
			return -EFAULT;
		}	
		len -= to_write;
		total_writed += to_write;
	}	

	return total_writed;
}

static int dnwbulk_open(struct inode *node, struct file *file)
{
	struct dnwbulk_dev *dev;
	struct usb_interface *interface;
	
	interface = usb_find_interface(&dnwbulk_driver, iminor(node));
	if(!interface)
		return -ENODEV;

	dev = usb_get_intfdata(interface);
	dev->bulkout_buffer = kzalloc(BULKOUT_BUFFER_SIZE, GFP_KERNEL);
	if(!(dev->bulkout_buffer))
		return -ENOMEM;
	if(!mutex_trylock(&dev->io_mutex))
		return -EBUSY;
	file->private_data = dev;

	return 0;
}

static int dnwbulk_release(struct inode *node, struct file *file)
{
	struct dnwbulk_dev *dev;

	dev = (struct dnwbulk_dev*)(file->private_data);
	kfree(dev->bulkout_buffer);
	mutex_unlock(&dev->io_mutex);

	return 0;
}

static struct file_operations dnwbulk_fops = {
	.owner 	=	THIS_MODULE,
	.read 	=	dnwbulk_read,
	.write	=	dnwbulk_write,
	.open   =	dnwbulk_open,
	.release=	dnwbulk_release,
};

static struct usb_class_driver dnwbulk_class = {
	.name = 		"dnwbulk%d",
	.fops =			&dnwbulk_fops,
	.minor_base=	100,
};

static int dnwbulk_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int i;
	int ret;
	struct dnwbulk_dev *dev;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	
	printk(KERN_INFO "dnwbulk:dnwbulk probing...\n");
	
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if(!dev)
	{
		ret = -ENOMEM;
		goto error;
	}	

	iface_desc = interface->cur_altsetting;
	for(i=0; i < iface_desc->desc.bNumEndpoints; i++)
	{
		endpoint = &(iface_desc->endpoint[i].desc);
		if(!dev->bulk_out_endpointAddr
			&& usb_endpoint_is_bulk_out(endpoint))
		{
			printk(KERN_INFO "dnwbulk:bulk out endpoint found!\n");
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
			break;
		}
	}
	
	if(!(dev->bulk_out_endpointAddr))
	{
		ret = -EBUSY;
		goto error;
	}

	ret = usb_register_dev(interface, &dnwbulk_class);
	if(ret)
	{
		printk(KERN_ERR "dnwbulk: usb_register_dev failed!\n");
		return ret;
	}		
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	usb_set_intfdata(interface, dev);	
	mutex_init(&dev->io_mutex);

	return 0;

error:
	if(!dev)
		kfree(dev);

	return ret;
}

static struct usb_driver dnwbulk_driver= {
	.name=			"dnwbulk",
	.probe=			dnwbulk_probe,
	.disconnect=	dnwbulk_disconnect,
	.id_table=		dnwbulk_table,
};

static int __init dnwbulk_init(void)
{
	int result;

	printk(KERN_INFO "dnwbulk:dnwbulk loaded\n");
	result = usb_register(&dnwbulk_driver);
	if(result)
	{	printk(KERN_ERR "dnwbulk:usb_register failed: %d", result);
		return result;
	}	
	
	return 0;
}

static void __exit dnwbulk_exit(void)
{
	usb_deregister(&dnwbulk_driver);
	printk(KERN_INFO "dnwbulk:dnwbulk unloaded\n");
}

module_init(dnwbulk_init);
module_exit(dnwbulk_exit);
MODULE_LICENSE("GPL");



