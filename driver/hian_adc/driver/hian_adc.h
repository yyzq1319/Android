#ifndef _HIAN_ADC_H_
#define _HIAN_ADC_H_

#ifndef DEVICE_NAME
#define DEVICE_NAME "hian_adc"
#endif

#define DRIVER_NAME "hian_adc"

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
#define EXYNOS4_PA_ADC    0x126C0000
#define EXYNOS4_SZ_ADC    SZ_4K
#define EXYNOS4_PA_IESR   0x10440000
#define IESR2             0x0020
#define ADCCON      0x0000
#define ADCDLY      0x0008
#define ADCDAT      0x000C
#define ADCCLEARINT 0x0018
#define ADCMUX      0x001C

struct adc_st{
	int value; //adc转换结果

	spinlock_t lock;
	int lock_count;
	
	struct cdev dev;
	dev_t no;

	unsigned int adccon;
	unsigned int adcdat;
	unsigned int adcdly;
	unsigned int clrintadc;
	
	void *__iomem v;
	void *__iomem v_iesr;
	
	struct clk *clk;
	struct clk *clkphy;
	struct clk *fsys_clk;
	
	int irq;
	wait_queue_head_t rq;
	
	bool flag;
};

struct adc_value_st{
	unsigned int value;
};
#endif
