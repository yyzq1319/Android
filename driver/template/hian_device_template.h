#ifndef _HIAN_DEVICE_TEMPLATE_H_
#define _HIAN_DEVICE_TEMPLATE_H_

#define GPIO_GPC0_3 0
#define GPIO_GPX0_6 1

#ifndef DEVICE_NAME
#define DEVICE_NAME "hian_deviceName"
#endif

#ifndef DEVICE_MINOR_NUM
#define DEVICE_MINOR_NUM 2
#endif

#ifndef DEV_MAJOR
#define DEV_MAJOR 0
#endif

#ifndef DEV_MINOR
#define DEV_MINOR 0
#endif

#ifndef REGDEV_SIZE
#define REGDEV_SIZE 3000
#endif

struct reg_dev
{
	char *data;
	unsigned long size;
	
	struct cdev cdev;
};
#endif
