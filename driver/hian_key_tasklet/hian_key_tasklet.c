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
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <mach/regs-gpio.h>
#include <linux/interrupt.h>
/*自定义头文件*/
//#include "hian_key_irq.h"

/*Linux中申请GPIO的头文件*/
#include <linux/gpio.h>
/*三星平台的GPIO配置函数头文件*/
/*三星平台EXYNOS系列平台，GPIO配置参数宏定义头文件*/
#include <plat/gpio-cfg.h>
/*三星平台4412平台，GPIO宏定义头文件*/
#include <mach/gpio-exynos4.h>

#define IRQ_DEBUG

#ifdef IRQ_DEBUG
#define DPRINTK(x...)printk("HIAN_KEY_IRQ_TASKLET_DEBUG:"x);
#else
#define DPRINTK(x...)
#endif

#define DRIVER_NAME "hian_key_irq"

struct tasklet_struct eint9_ts;
struct tasklet_struct eint10_ts;

unsigned long data = 0;

static void eint9_tasklet_isr(unsigned long data)
{
	printk("interrupt9 tasklet.......!!!\r\n");
	return;	
}

static void eint10_tasklet_isr(unsigned long data)
{
	printk("interrupt10 tasklet.......!!!\r\n");
	return;	
}

static irqreturn_t hian_key_eint9_interrupt(int irq,void *dev_id)
{
	printk("enter interrupt9!!!\r\n");

	tasklet_schedule(&eint9_ts);
	
	return IRQ_HANDLED;
}

static irqreturn_t hian_key_eint10_interrupt(int irq,void *dev_id)
{
	printk("enter interrupt10!!!\r\n");
	
	tasklet_schedule(&eint10_ts);
	
	return IRQ_HANDLED;
}

/*中断函数注册 */
static int hian_key_irq_probe(struct platform_device *pdev)
{
	int ret = 0;
	
	char *banner = "hian_key_irq_test Initialize\n";
	printk(banner);

	/*设置按键GPIO为中断模式 */
	ret = gpio_request(EXYNOS4_GPX1(1),"EINT9");
	if(ret)
	{
		printk("%s:request GPIO%d for EINT9 failed!,ret = %d\n", DRIVER_NAME,EXYNOS4_GPX1(1), ret);
		
		return -EFAULT;
	}
	
	/* 0xf为中断模式 */
	s3c_gpio_cfgpin(EXYNOS4_GPX1(1),S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(EXYNOS4_GPX1(1),S3C_GPIO_PULL_UP);
	gpio_free(EXYNOS4_GPX1(1));
	
	/*设置按键GPIO为中断模式 */
	ret = gpio_request(EXYNOS4_GPX1(2),"EINT10");
	if(ret)
	{
		printk("%s:request GPIO%d for EINT10 failed!,ret = %d\n", DRIVER_NAME, EXYNOS4_GPX1(2), ret);
		
		return -EFAULT;
	}
	
	/* 0xf为中断模式 */
	s3c_gpio_cfgpin(EXYNOS4_GPX1(2),S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(EXYNOS4_GPX1(2),S3C_GPIO_PULL_UP);
	gpio_free(EXYNOS4_GPX1(2));
	
	/*申请中断 */
	ret = request_irq(IRQ_EINT(9), hian_key_eint9_interrupt, IRQ_TYPE_EDGE_FALLING, "eint9", pdev);
	if(ret < 0)
	{
		printk("Request IRQ %d failed,%d\n",IRQ_EINT(9), ret);
		goto exit;
	}

	ret = request_irq(IRQ_EINT(10), hian_key_eint10_interrupt, IRQ_TYPE_EDGE_FALLING, "eint10", pdev);
	if(ret < 0)
	{
		printk("Request IRQ %d failed,%d\n",IRQ_EINT(10),ret);
		goto exit;
	}
	
	tasklet_init(&eint9_ts, eint9_tasklet_isr, data);	
	tasklet_init(&eint10_ts, eint10_tasklet_isr, data);	

	return 0;	

	exit:
	return -EFAULT;
}

static int hian_key_irq_remove(struct platform_device *pdev)
{
	free_irq(IRQ_EINT(9), pdev);
	free_irq(IRQ_EINT(10), pdev);

	return 0;
}

static int hian_key_irq_suspend(struct platform_device *pdev, pm_message_t state)
{
	DPRINTK("irq suspend:power off!\n");
	return 0;
}

static struct platform_driver hian_key_irq_driver = {
	.probe = hian_key_irq_probe,
	.remove = hian_key_irq_remove,
	.suspend = hian_key_irq_suspend,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static void __exit hian_key_irq_test_exit(void)
{
	platform_driver_unregister(&hian_key_irq_driver);
}

static int __init hian_key_irq_test_init(void)
{
	return platform_driver_register(&hian_key_irq_driver);
}

module_init(hian_key_irq_test_init);
module_exit(hian_key_irq_test_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

