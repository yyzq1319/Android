#ifndef _HIAN_DEBUG_H_
#define _HIAN_DEBUG_H_

struct led_bus{
	int id;
	struct bus_type bus;
};

struct led_device{
	int id;
	struct device dev;
};

struct led_driver{
	int id;
	struct device_driver drv;
};

#endif
