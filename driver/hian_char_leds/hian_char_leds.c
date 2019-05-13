/*包含初始化宏定义的头文件,代码中的module_init和module_exit在此文件中*/
#include <linux/init.h>
/*包含初始化加载模块的头文件,代码中的MODULE_LICENSE在此头文件中*/
#include <linux/module.h>
/*定义module_param module_param_array的头文件*/
#include <linux/moduleparam.h>
/*定义module_param module_param_array中perm的头文件*/
#include <linux/stat.h>
/*三个字符设备函数*/
#include <linux/fs.h>
/*MKDEV转换设备号数据类型的宏定义*/
#include <linux/kdev_t.h>
/*定义字符设备的结构体*/
#include <linux/cdev.h>
/*分配内存空间函数头文件*/
#include <linux/slab.h>
/*包含函数device_create 结构体class等头文件*/
#include <linux/device.h>

/*自定义头文件*/
#include "hian_char_leds.h"

/*Linux中申请GPIO的头文件*/
#include <linux/gpio.h>
/*三星平台的GPIO配置函数头文件*/
/*三星平台EXYNOS系列平台，GPIO配置参数宏定义头文件*/
#include <plat/gpio-cfg.h>
/*三星平台4412平台，GPIO宏定义头文件*/
#include <mach/gpio-exynos4.h>

#define DRIVER_NAME "hian_leds"
#define DEVICE_NAME "hian_leds"
#define FRIST_LED 0
#define SECOND_LED 1
#define LED_ON 1
#define LED_OFF 0

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

static hian_led_gpios[] = {
	EXYNOS4_GPL2(0),
	EXYNOS4_GPK1(1),
};

#define LED_NUM ARRAY_SIZE(hian_led_gpios)
int numdev_major = DEV_MAJOR;
int numdev_minor = DEV_MINOR;

/*输入主设备号 */
module_param(numdev_major,int,S_IRUSR);
/*输入次设备号 */
module_param(numdev_minor,int,S_IRUSR);

static struct class *hian_class;
struct reg_dev *hian_devices;

static long hian_leds_ioctl( struct file *files, unsigned int cmd, unsigned long num)
{
	int ret = 0;
	
	if(num > LED_NUM)
	{
		printk("input leds num arg error\n");
		return -1;		
	}
	
	if((cmd != LED_ON ) && (cmd != LED_OFF))
	{
		printk("input leds cmd arg error\n");
		return -1;		
	}

	if(cmd == 0)
	{
		gpio_set_value(hian_led_gpios[num],LED_ON);				
    }
	else if(cmd == 1)
	{
		gpio_set_value(hian_led_gpios[num],LED_OFF);				
	}

	return 0;
}

static int hian_leds_release(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_leds release\n");
	return 0;
}

static int hian_leds_open(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_leds open\n");
	return nonseekable_open(inode,file);
}

static struct file_operations hian_leds_ops = {
	.owner = THIS_MODULE,
	.open = hian_leds_open,
	.release = hian_leds_release,
	.unlocked_ioctl = hian_leds_ioctl,
};


static int hian_led_gpios_init(unsigned int num)
{
	int ret = 0;

	ret = gpio_request(hian_led_gpios[num],"HIAN_FRIST_LED");
	if(ret < 0)
	{
		printk(KERN_EMERG "gpio_request %s led failed!\n",hian_led_gpios[num]);
		return ret;
	}
	
	s3c_gpio_cfgpin(hian_led_gpios[num],S3C_GPIO_OUTPUT);
	gpio_set_value(hian_led_gpios[num],LED_OFF);

	return 0;	
}

static void reg_init_cdev(struct reg_dev *dev,int index)
{
	int err;
	int devno = MKDEV(numdev_major,numdev_minor+index);//获取设备号

	/*数据初始化*/
	cdev_init(&dev->cdev,&hian_leds_ops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &hian_leds_ops;

	/*注册驱动到系统*/
	err = cdev_add(&dev->cdev,devno,1);
	if(err)
	{
		printk(KERN_EMERG "cdev_add %d is fail! %d\n",index,err);
	}
	else
	{
		printk(KERN_EMERG "cdev_add %d is success!\n",numdev_minor+index);
	}
}

static int scdev_init(void)
{
	int ret = 0,i;
	dev_t num_dev;

	printk(KERN_EMERG "numdev_major is %d!\n",numdev_major);
	printk(KERN_EMERG "numdev_minor is %d!\n",numdev_minor);

	if(numdev_major)
	{
		/*静态注册设备号*/
		num_dev = MKDEV(numdev_major,numdev_minor);
		ret = register_chrdev_region(num_dev,DEVICE_MINOR_NUM,DEVICE_NAME);
	}
	else
	{
		/*动态注册设备号*/
		ret = alloc_chrdev_region(&num_dev,numdev_minor,DEVICE_MINOR_NUM,DEVICE_NAME);

		/*获得主设备号*/
		numdev_major = MAJOR(num_dev);
		printk(KERN_EMERG "adev_region req %d !\n",numdev_major);
	}

	if(ret<0)
	{
		printk(KERN_EMERG "register_chrdev_region req %d is failed!\n",numdev_major);	
	}

	hian_class = class_create(THIS_MODULE,DEVICE_NAME);
	hian_devices = kmalloc(DEVICE_MINOR_NUM * sizeof(struct reg_dev),GFP_KERNEL);

	if(!hian_devices)
	{
		ret = -ENOMEM;
		goto fail;
	}

	memset(hian_devices,0,DEVICE_MINOR_NUM * sizeof(struct reg_dev));

	/*设备初始化*/
	for(i=0;i<DEVICE_MINOR_NUM;i++)
	{

		hian_devices[i].data = kmalloc(REGDEV_SIZE,GFP_KERNEL);
		memset(hian_devices[i].data,0,REGDEV_SIZE);

		/*设备注册到系统*/
		reg_init_cdev(&hian_devices[i],i);

		/*创建设备节点*/
		device_create(hian_class,NULL,MKDEV(numdev_major,numdev_minor+i),NULL,DEVICE_NAME"%d",i);
	}

	for(i=0; i<LED_NUM; i++)
	{
		ret = hian_led_gpios_init(i);
		if(ret)
		{
			printk(KERN_EMERG "gpio_init failed!\n");
		}	
	}

	printk(KERN_EMERG "scdev_init!\n");
	/*打印信息，KERN_EMERG表示紧急信息*/
	return 0;

fail:
	/*注销设备号*/
	unregister_chrdev_region(MKDEV(numdev_major,numdev_minor),DEVICE_MINOR_NUM);
	printk(KERN_EMERG "kmalloc is fail!\n");

	return ret;
}

static void scdev_exit(void)
{

	int i;
	printk(KERN_EMERG "scdev_exit!\n");

	/*除去字符设备*/
	for(i=0;i<DEVICE_MINOR_NUM;i++)
	{
		cdev_del(&(hian_devices[i].cdev));
		/*摧毁设备节点函数d*/
		device_destroy(hian_class,MKDEV(numdev_major,numdev_minor+i));
	}

	/*释放设备class*/
	class_destroy(hian_class);

	/*释放内存*/
	kfree(hian_devices);

	/*释放GPIO*/
	for(i=0;i<LED_NUM;i++)
	{
		gpio_free(hian_led_gpios[i]);
	}

	unregister_chrdev_region(MKDEV(numdev_major,numdev_minor),DEVICE_MINOR_NUM);
}

module_init(scdev_init);
/*初始化函数*/
module_exit(scdev_exit);
/*卸载函数*/
