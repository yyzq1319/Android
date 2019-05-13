#include <linux/init.h>
#include <linux/module.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
/*三个字符设备函数*/
#include <linux/fs.h>
/*MKDEV转换设备号数据类型的宏定义*/
#include<linux/kdev_t.h>
/*定义字符设备的结构体*/
#include<linux/cdev.h>
/*定义module_param_array的头文件*/
#include <linux/moduleparam.h>
/*定义module_param module_param_array函数中perm的头文件*/
#include <linux/stat.h>

#define DEVICE_NAME "hian_ascdev"
#define DEVICE_MINOR_NUM 2
#define DEV_MAJOR 0
#define DEV_MINOR 0

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

int numdev_major = DEV_MAJOR;
int numdev_minor = DEV_MINOR;

/*输入主次设备号 */
module_param(numdev_major, int, S_IRUSR);
module_param(numdev_minor, int, S_IRUSR);

static int hian_ascdev_init(void)
{
	int ret = 0;
	dev_t num_dev;

	printk(KERN_EMERG "numdev_major is %d!\n",numdev_major);		
	printk(KERN_EMERG "numdev_minor is %d!\n",numdev_minor);

	/*字符设备注册*/		
	if(numdev_major)
	{
		num_dev = MKDEV(numdev_major,numdev_minor);
		ret = register_chrdev_region(num_dev,DEVICE_MINOR_NUM,DEVICE_NAME);	
	}
	else
	{
		/*动态申请设备号*/
		ret = alloc_chrdev_region(&num_dev,numdev_minor,DEVICE_MINOR_NUM,DEVICE_NAME);
		/*获取主设备号*/
		numdev_major = MAJOR(num_dev);
		printk(KERN_EMERG "numdev_major is %d!\n",numdev_major);
		
	}
	
	if(ret < 0)
	{
		printk(KERN_EMERG "register_chrdev_regin req %d is failed!\n",ret);
			
	}

	printk(KERN_EMERG "HIAN CCSDEV INIT!\n");
	
	return 0;
}


static void hian_ascdev_exit(void)
{
	printk(KERN_EMERG "HIAN ASCDEV exit!\n");
	
	unregister_chrdev_region(MKDEV(numdev_major,numdev_minor),DEVICE_MINOR_NUM);	
}

module_init(hian_ascdev_init);
module_exit(hian_ascdev_exit);
