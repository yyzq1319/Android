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
#include "hian_gpio_input.h"

/*Linux中申请GPIO的头文件*/
#include <linux/gpio.h>
/*三星平台的GPIO配置函数头文件*/
/*三星平台EXYNOS系列平台，GPIO配置参数宏定义头文件*/
#include <plat/gpio-cfg.h>
/*三星平台4412平台，GPIO宏定义头文件*/
#include <mach/gpio-exynos4.h>

#define DRIVER_NAME "hian_gpio_input"
#define DEVICE_NAME "hian_gpio_input"
int numdev_major = DEV_MAJOR;
int numdev_minor = DEV_MINOR;

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

static hian_gpios[] = {
	EXYNOS4_GPC0(3),
	EXYNOS4_GPX0(6),
};

static char *hian_gpio_name[] = {
	"SWITCH3",
	"SWITCH4",
};

#define GPIO_NUM ARRAY_SIZE(hian_gpios)

static struct class *hian_class;
struct reg_dev *hian_devices;

static long hian_read_gpio_ioctl( struct file *files, unsigned int cmd, unsigned long num)
{
	if(num > GPIO_NUM)
	{
		printk("input gpios num arg error\n");
		return -1;		
	}
	
	if((cmd != GPIO_GPC0_3 ) && (cmd != GPIO_GPX0_6))
	{
		printk("input gpios cmd arg error\n");
		return -1;		
	}

	if(cmd == 0)
	{
		return gpio_get_value(EXYNOS4_GPC0(3));				
    }
	else if(cmd == 1)
	{
		return gpio_get_value(EXYNOS4_GPX0(6));				
	}

	return 0;
}

static int hian_read_gpio_release(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_read_gpio release\n");
	return 0;
}

static int hian_read_gpio_open(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_read_gpio open\n");
	return nonseekable_open(inode,file);
}

static struct file_operations hian_read_gpio_ops = {
	.owner = THIS_MODULE,
	.open = hian_read_gpio_open,
	.release = hian_read_gpio_release,
	.unlocked_ioctl = hian_read_gpio_ioctl,
};


static int hian_read_gpios_init(unsigned int num)
{
	int ret = 0;

	ret = gpio_request(hian_gpios[num],hian_gpio_name[num]);
	if(ret < 0)
	{
		printk(KERN_EMERG "gpio_request %s  failed!\n",hian_gpio_name[num]);
		return ret;
	}
	
	s3c_gpio_cfgpin(hian_gpios[num],S3C_GPIO_INPUT);
	s3c_gpio_setpull(hian_gpios[num],S3C_GPIO_PULL_NONE);

	return 0;	
}

static void reg_init_cdev(struct reg_dev *dev,int index)
{
	int err;
	int devno = MKDEV(numdev_major,numdev_minor+index);//获取设备号

	/*数据初始化*/
	cdev_init(&dev->cdev,&hian_read_gpio_ops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &hian_read_gpio_ops;

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

	/*动态注册设备号*/
	ret = alloc_chrdev_region(&num_dev,numdev_minor,DEVICE_MINOR_NUM,DEVICE_NAME);

	/*获得主设备号*/
	numdev_major = MAJOR(num_dev);
	printk(KERN_EMERG "adev_region req %d !\n",numdev_major);

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

	for(i=0; i<GPIO_NUM; i++)
	{
		ret = hian_read_gpios_init(i);
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
	for(i=0;i<GPIO_NUM;i++)
	{
		gpio_free(hian_gpios[i]);
	}

	unregister_chrdev_region(MKDEV(numdev_major,numdev_minor),DEVICE_MINOR_NUM);
}

module_init(scdev_init);
/*初始化函数*/
module_exit(scdev_exit);
/*卸载函数*/
