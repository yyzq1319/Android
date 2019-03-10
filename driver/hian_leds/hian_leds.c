#include <linux/init.h>
#include <linux/module.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
/*注册设备节点的文件结构体*/
#include <linux/fs.h>
/*Linux中申请GPIO的头文件*/
#include <linux/gpio.h>
/*三星平台的GPIO配置函数头文件*/
/*三星平台EXYNOS系列平台，GPIO配置参数宏定义头文件*/
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
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

static  struct miscdevice hian_leds_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &hian_leds_ops,
};

static int hian_led_gpios_init(unsigned int num)
{
	int ret = 0;

	ret = gpio_request(hian_led_gpios[num],"HIAN_FRIST_LED");
	if(ret < 0){
		printk(KERN_EMERG "gpio_request %s led failed!\n",hian_led_gpios[num]);
		return ret;
	}
	
	s3c_gpio_cfgpin(hian_led_gpios[num],S3C_GPIO_OUTPUT);
	
	gpio_set_value(hian_led_gpios[num],LED_OFF);

	return 0;	
}

static int hian_leds_probe(struct platform_device *pdv){
	int ret;
	unsigned int num = 0;
	
	printk(KERN_EMERG "\tinitialized\n");
	
	for(num = 0; num < LED_NUM; num++)
	{
		hian_led_gpios_init(num);
	}
	
	misc_register(&hian_leds_dev);
	
	return 0;
}

static int hian_leds_remove(struct platform_device *pdv){
	
	printk(KERN_EMERG "\thian led remove\n");
	misc_deregister(&hian_leds_dev);
	return 0;
}

static void hian_leds_shutdown(struct platform_device *pdv){
	
	;
}

static int hian_leds_suspend(struct platform_device *pdv,pm_message_t pmt){
	
	return 0;
}

static int hian_leds_resume(struct platform_device *pdv){
	
	return 0;
}

struct platform_driver hian_leds_driver = {
	.probe = hian_leds_probe,
	.remove = hian_leds_remove,
	.shutdown = hian_leds_shutdown,
	.suspend = hian_leds_suspend,
	.resume = hian_leds_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	}
};


static int hian_leds_init(void)
{
	int DriverState;
	
	printk(KERN_EMERG "HIAN LEDS enter!\n");
	DriverState = platform_driver_register(&hian_leds_driver);
	
	printk(KERN_EMERG "\tDriverState is %d\n",DriverState);
	return 0;
}


static void hian_leds_exit(void)
{
	printk(KERN_EMERG "HIAN LEDS exit!\n");
	
	platform_driver_unregister(&hian_leds_driver);	
}

module_init(hian_leds_init);
module_exit(hian_leds_exit);
