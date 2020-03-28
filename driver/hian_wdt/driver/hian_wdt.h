#ifndef _HIAN_WDT_H_
#define _HIAN_WDT_H_

#define GPIO_GPC0_3 0
#define GPIO_GPX0_6 1

#ifndef DEVICE_NAME
#define DEVICE_NAME "hian_wdt"
#endif

#define DRIVER_NAME "hian_wdt"

#ifndef DEVICE_MINOR_NUM
#define DEVICE_MINOR_NUM 1
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

#define EXYNOS4_PA_WDT 0x10060000
#define WTCON 0x00
#define WTCNT 0x08

struct wdt_st{
	int wdtcon, wdtdat, wdtcnt;
	void *__iomem v;
	struct clk *clk;
	
	struct cdev dev;
	
	spinlock_t lock;
	int count;
};

struct wdt_feed_st{
	unsigned int times;
};
#endif
