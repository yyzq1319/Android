#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/sched.h>
#include "hian_debug.h"

int led_match(struct device *dev, struct device_driver *drv);

struct led_bus ledbus = {
	.id = 0,
	.bus = {
				.name = "ledbus",
				.match = led_match,
		},
};

int led_match(struct device *dev, struct device_driver *drv)
{
	return 1;
}
static __init int scdev_init(void)
{
	return bus_register(&ledbus.bus);
}

static __exit void scdev_exit(void)
{
	bus_unregister(&ledbus.bus);
}

MODULE_LICENSE("GPL");
module_init(scdev_init);
/*初始化函数*/
module_exit(scdev_exit);
/*卸载函数*/
