#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/sched.h>
#include "hian_debug.h"

int led_match(struct device *dev, struct device_driver *drv);
void led_release(struct device *dev);
int led_probe(struct device *dev);
int led_remove(struct device *dev);

struct led_bus ledbus = {
	.id = 0,
	.bus = {
				.name = "ledbus",
				.match = led_match,
		},
};

struct led_device red = {
	.id = RED,
	.dev = {
		.init_name = "reddev",
		.bus = &ledbus.bus,
		.release = led_release,
	}
};

struct led_device green = {
	.id = GREEN,
	.dev = {
		.init_name = "greendev",
		.bus = &ledbus.bus,
		.release = led_release,
	}
};

struct led_device blue = {
	.id = BLUE,
	.dev = {
		.init_name = "bluedev",
		.bus = &ledbus.bus,
		.release = led_release,
	}
};

struct led_driver redDrv = {
	.id = RED,
	.drv = {
		.name = "reddrv",
		.bus = &ledbus.bus,
		.probe = led_probe,
		.remove = led_remove,
	}
};

int led_match(struct device *pdev, struct device_driver *pdrv)
{
	struct led_device *ldev;
	struct led_driver *ldrv;
	
	ldev = container_of(pdev, struct led_device, dev);
	ldrv = container_of(pdrv, struct led_driver, drv);

	
	return (ldev->id == ldrv->id);
}

void led_release(struct device *dev)
{
	printk("device unregister \r\n");
}


int led_probe(struct device *pdev)
{
	struct led_device *ldev;
	ldev = container_of(pdev, struct led_device, dev);
	
	switch(ldev->id)
	{
		case RED:
			printk("red match me! \r\n");
			break;
		case GREEN:
			printk("green match me! \r\n");
			break;
		case BLUE:
			printk("blue match me! \r\n");
			break;
		default:
			break;
	}
	
	return 0;
}

int led_remove(struct device *pdev)
{
	struct led_device *ldev;
	ldev = container_of(pdev, struct led_device, dev);
	
	switch(ldev->id)
	{
		case RED:
			printk("red leave me! \r\n");
			break;
		case GREEN:
			printk("green leave me! \r\n");
			break;
		case BLUE:
			printk("blue leave me! \r\n");
			break;
		default:
			break;
	}	
	return 0;
}

static __init int scdev_init(void)
{
	int ret;
	
	ret |= bus_register(&ledbus.bus);
	ret |= device_register(&red.dev);
	ret |= device_register(&green.dev);
	ret |= device_register(&blue.dev);
	ret |= driver_register(&redDrv.drv);

	return ret;
}

static __exit void scdev_exit(void)
{
	driver_unregister(&redDrv.drv);
	device_unregister(&red.dev);
	device_unregister(&green.dev);
	device_unregister(&blue.dev);
	bus_unregister(&ledbus.bus);
}

MODULE_LICENSE("GPL");
module_init(scdev_init);
/*初始化函数*/
module_exit(scdev_exit);
/*卸载函数*/
