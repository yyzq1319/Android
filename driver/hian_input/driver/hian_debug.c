#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <plat/irqs.h>
#include <linux/interrupt.h>
#include <linux/input.h>
 
struct mill_key{
	int code;//定义按键编码
	int irqnum;
	char *name;
	u32 cnt;
};

struct mill_key millkeys[] = {
	{KEY_UP,    IRQ_EINT(10), "key1", 0},
	{KEY_DOWN,    IRQ_EINT(27), "key2", 0},
	{KEY_LEFT,    IRQ_EINT(16), "key3", 0},
	{KEY_RIGHT,    IRQ_EINT(17), "key4", 0},
};
 
 /*1st*/
static struct input_dev *funckeys;//1.定义一个输入设备的地址
 
static struct tasklet_struct task;

static void mill_unregister_irqkey(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(millkeys); ++i) 
	{
		free_irq(millkeys[i].irqnum, &millkeys[i]);
	}
     
}
 
/*irq bootm half*/
static void do_bh_handler (unsigned long data)
{
	struct mill_key *ptr = (void *)data;    
	ptr->cnt++;
	printk("event->code %d!\r\n", ptr->code);
	
	input_report_key(funckeys, ptr->code, ptr->cnt%2);//按键事件的封装并上报该按键事件
 	input_sync(funckeys);//每一时刻读完数据上报一个同步事件，全部上报完了再上报一个同步事件，从时间上区分
}
 
/*irq top half*/
static irqreturn_t do_handler(int irqnum, void *dev)
{
	task.data = (unsigned long)dev;

	tasklet_schedule(&task);    

	return IRQ_HANDLED;
}
 
static int mill_register_irqkey(void)
{
	int i;
	int ret;
	
	for (i = 0; i < ARRAY_SIZE(millkeys); ++i) 
	{
		ret = request_irq(millkeys[i].irqnum, do_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
             millkeys[i].name, &millkeys[i]);
		if (ret < 0) 
		{
	     	goto error0;
		}
	}
 
	return 0;
error0:
	while (i--) 
	{
		free_irq(millkeys[i].irqnum, &millkeys[i]);
	}

	return ret;

}
 
 /* driver module entry */
static int __init demo_init(void)
{
	int code, ret;
	int i;
 
	/*2nd*/
	funckeys = input_allocate_device();//2.为该输入设备分配地址空间
	if (!funckeys) 
	{//申请失败则返回错误负值
		return -ENOMEM;
	}
 
 	/*3rd*/
	//#define EV_KEY            0x01    //按键
	funckeys->evbit[BIT_WORD(EV_KEY)] |= BIT_MASK(EV_KEY);
	//3.从大的事件分类中获取按键这个事件分类并取按键的编码/
	//                    1/32 == 0        1<<(1%32) == 1位置1
	for (i = 0; i < ARRAY_SIZE(millkeys); ++i) 
	{
		code = millkeys[i].code;
		funckeys->keybit[BIT_WORD(code)] |= BIT_MASK(code);//3.依次获取4个按键的编码
		//                    103/32 == 3        1<<(103%32) == 7位置1      
	}
	/*
	#define KEY_UP            103
	#define KEY_LEFT        105
	#define KEY_RIGHT        106
	#define KEY_DOWN        108	
	*/

	/*4th: 向内核注册输入字符设备驱动*/
	ret = input_register_device(funckeys);//4.向内核注册该输入字符设备驱动
	if (ret < 0) 
	{
		goto error0;
	}

	tasklet_init(&task, do_bh_handler, 0);

	ret = mill_register_irqkey();
	if (ret < 0) 
	{
		goto error1;
	}

	printk("funckeys device is ok!\n");

	return 0;
error1:
	input_unregister_device(funckeys);
	    return ret;
	error0:
	    input_free_device(funckeys);    
	return ret;
}
 
module_init(demo_init);

 
/* driver module exit */
static void __exit demo_exit(void)
{
	tasklet_kill(&task);

	mill_unregister_irqkey();

	input_unregister_device(funckeys);
}

module_exit(demo_exit);
 
/* driver module description */
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("crmn");
MODULE_VERSION("crmn1.0");
MODULE_DESCRIPTION("example for driver module arch");
