#ifndef _HIAN_DEBUG_H_
#define _HIAN_DEBUG_H_

#define GPX3DAT 0x0C64
#define EXYNOS4_PA_BUTTON 0x11000000
#defien EXYNOS4_NUM_BUTTOM 4

struct button_st{
	void *__iomem v;
	int irq;
	int code;  //up down back enter

	struct input_dev *dev;
};


#endif
