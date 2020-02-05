/*模块头文件*/
#include <linux/init.h>
#include <linux/module.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
/*注册设备节点的文件结构体*/
#include <linux/fs.h>
/*其他需要头文件*/

#define DRIVER_NAME "hian_deviceName"
#define DEVICE_NAME "hian_deviceName"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");


static int hian_deviceName_open(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_deviceName open in!\n");	
	...	
	printk(KERN_EMERG "hian_deviceName open success!\n");	
	return 0;
}

static int hian_deviceName_release(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_deviceName release\n");
	...	
	return 0;
}

static struct file_operations hian_deviceName_ops = {
	.owner = THIS_MODULE,
	.open = hian_deviceName_open,
	.release = hian_deviceName_release,
};


static  struct miscdevice hian_deviceName_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &hian_deviceName_ops,
};


static int hian_deviceName_probe(struct platform_device *pdv)
{
	
	printk(KERN_EMERG "\thian_deviceName initialized\n");
	misc_register(&hian_deviceName_dev);
	
	return 0;
}

static int hian_deviceName_remove(struct platform_device *pdv)
{
	
	printk(KERN_EMERG "\thian_deviceName remove\n");
	misc_deregister(&hian_deviceName_dev);
	return 0;
}

struct platform_driver hian_deviceName_driver = {
	.probe = hian_deviceName_probe,
	.remove = hian_deviceName_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	}
};


static int hian_deviceName_init(void)
{
	int DriverState;
	
	printk(KERN_EMERG "hian_deviceName enter!\n");
	DriverState = platform_driver_register(&hian_deviceName_driver);
	
	printk(KERN_EMERG "\thian_deviceName DriverState is %d\n",DriverState);
	return 0;
}


static void hian_deviceName_exit(void)
{
	printk(KERN_EMERG "hian_deviceName exit!\n");
	
	platform_driver_unregister(&hian_deviceName_driver);	
}

module_init(hian_deviceName_init);
module_exit(hian_deviceName_exit);
