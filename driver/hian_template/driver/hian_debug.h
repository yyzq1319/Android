#ifndef _HIAN_DEBUG_H_
#define _HIAN_DEBUG_H_

#ifndef NO_COMPILE
#define NO_COMPILE 0
#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME "hian_debug"
#endif

#define DRIVER_NAME "hian_debug"

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

#define HAVE_DATA   0
#define NO_DATA     1

struct debug_st{
	int value; //adc转换结果

	spinlock_t lock;
	int lock_count;
	
	struct cdev dev;
	dev_t no;

	void *__iomem v;
	
	struct clk *clk;
	
	int irq;
	wait_queue_head_t rq;
	
	bool flag;
};

struct debug_value_st{
	unsigned int value;
};
#endif
