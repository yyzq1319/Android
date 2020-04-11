#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include <linux/sched.h>
#include <linux/interrupt.h> 
#include <linux/irq.h>       
#include <linux/wait.h>
#include <mach/irqs.h>
#include <linux/poll.h>
#include <linux/hrtimer.h>
#include "hian_debug.h"

int numdev_major = DEV_MAJOR;
int numdev_minor = DEV_MINOR;

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

struct class *hian_class;
struct debug_st *debug;

struct hrtimer timer;

enum hrtimer_restart timer_hander(struct hrtimer *t)
{
	printk(KERN_EMERG "hrtimer hander!\r\n");

	//相对时间，一次有效
//	return HRTIMER_NORESTART; //定时一次有效，不重启定时器

	//重启定时器
	//ktime_t tt = ktime_set(3, 3000000);
	//hrtimer_forward_now(t, tt);
	
//	return HRTIMER_RESTART;

	//绝对时间，一次有效
	return HRTIMER_NORESTART; //定时一次有效，不重启定时器
	
}

static int debug_init(struct debug_st *debug)
{
	return 0;
}

static int debug_exit(struct debug_st *debug)
{
	printk(KERN_EMERG "debug exit\n");
	
	return 0;
}

static void  debug_enable(struct debug_st *debug)
{
	printk(KERN_EMERG "debug enable!\r\n");
}

static void debug_disable(struct debug_st *debug)
{
	printk(KERN_EMERG "debug disable!\r\n");
}

static int get_data(struct debug_st *debug)
{
	int data = 0;

	return data;
}

void debug_clrInt(struct debug_st *debug)
{
	//清中断代码
}

irqreturn_t debug_isr(int irq, void *data)
{
	struct debug_st *debug;

	debug = data;

	printk(KERN_EMERG "debug irq isr......\r\n");
	debug->value = get_data(debug);
	debug->flag = HAVE_DATA;	

	wake_up(&(debug->rq));

	debug_clrInt(debug);

	return IRQ_HANDLED;
}

static long hian_debug_ioctl( struct file *fp, unsigned int cmd, unsigned long num)
{
	struct debug_st *debug;

	debug = fp->private_data;	
	switch(cmd)
	{
		case 0:
			debug_enable(debug);
			break;
		case 1:
			debug_disable(debug);
			break;
		default:
			break;
	}

	return 0;
}

static int hian_debug_open(struct inode *inode, struct file *fp)
{
	struct debug_st *debug;

	debug = container_of(inode->i_cdev, struct debug_st, dev);
	fp->private_data = debug;

	spin_lock(&debug->lock);
	if(debug->lock_count)
	{
		spin_unlock(&debug->lock);
		return -EBUSY;
	}
	debug->lock_count = 1;
	spin_unlock(&debug->lock);

	printk(KERN_EMERG "hian_debug open\n");
	
	return 0;
}

static int hian_debug_release(struct inode *inode, struct file *fp)
{
	struct debug_st *debug;
	
	debug = fp->private_data;
	spin_lock(&debug->lock);
	if(!(debug->lock_count)) //没有打开的设备
	{
		spin_unlock(&debug->lock);
		return -ENODEV;
	}
	debug->lock_count = 0;
	spin_unlock(&debug->lock);

	printk(KERN_EMERG "hian_debug release\n");

	return 0;
}
ssize_t hian_debug_read(struct file *fp, char __user *buffer, size_t count, loff_t *off)
{
	struct debug_st *debug;
	struct debug_value_st avs;
	int ret;

	debug = fp->private_data;

	if(count != sizeof(avs))
	{
		printk(KERN_EMERG "read is fail:count =  %d\n", count);
		return -EINVAL;
	}	

	while(debug->flag == NO_DATA)
	{
		if(fp->f_flags & O_NONBLOCK) //非阻塞访问
		{
			printk(KERN_EMERG "read is fail:O_NONBLOCK!\r\n");
			return -EAGAIN;
		}

		//阻塞访问	
		wait_event_interruptible(debug->rq, debug->flag == HAVE_DATA);
	}

	spin_lock(&debug->lock);
	avs.value = debug->value;
	ret = copy_to_user(buffer, &avs.value, sizeof(avs));
	if(ret)
	{
		printk(KERN_EMERG "read is fail:copy_to_user!\r\n");
		spin_unlock(&debug->lock);
		return -EFAULT;
	}
	spin_unlock(&debug->lock);
	
	debug->flag = NO_DATA;
	
	return count;

}

static int hian_debug_write(struct file *fp, const char *buffer, size_t count, loff_t *off)
{
	return count;
}

unsigned int hian_debug_poll(struct file *fp, struct poll_table_struct *table)
{
	struct debug_st *debug;
	unsigned int mask = 0;

	debug = fp->private_data;
	poll_wait(fp, &debug->rq, table);

	if(debug->flag == HAVE_DATA)
	{
		mask |= POLLIN;
	}
	
	return mask;
}

static struct file_operations hian_debug_ops = {
	.owner = THIS_MODULE,
	.open = hian_debug_open,
	.read = hian_debug_read,
	.write = hian_debug_write,
	.release = hian_debug_release,
	.unlocked_ioctl = hian_debug_ioctl,
	.poll = hian_debug_poll,
};


static void reg_init_cdev(struct debug_st *debug)
{
	int err;
	 debug->no = MKDEV(numdev_major, numdev_minor);//获取设备号

	/*数据初始化*/
	cdev_init(&debug->dev, &hian_debug_ops);
	debug->dev.owner = THIS_MODULE;
	debug->dev.ops = &hian_debug_ops;

	/*注册驱动到系统*/
	err = cdev_add(&debug->dev, debug->no, 1);
	if(err)
	{
		unregister_chrdev_region(MKDEV(numdev_major, numdev_minor), DEVICE_MINOR_NUM);
		printk(KERN_EMERG "cdev_add is fail! %d\n",err);
	}
	else
	{
		printk(KERN_EMERG "cdev_add %d is success!\n",numdev_minor);
	}
}

static __init int scdev_init(void)
{
	int ret = 0;

	debug = kzalloc(sizeof(struct debug_st), GFP_KERNEL);
	if(!debug)
	{
		ret = -ENOMEM;
		goto fail;
	}

	/*动态注册设备号*/
	ret = alloc_chrdev_region(&debug->no, 1, DEVICE_MINOR_NUM, DEVICE_NAME);

	/*获得主设备号*/
	numdev_major = MAJOR(debug->no);
	printk(KERN_EMERG "adev_region req %d !\n", numdev_major);
	if(ret<0)
	{
		printk(KERN_EMERG "register_chrdev_region req %d is failed!\n", numdev_major);	
	}

	hian_class = class_create(THIS_MODULE, DEVICE_NAME);

	/*设备初始化*/
	debug->flag = NO_DATA;

	/*设备注册到系统*/
	reg_init_cdev(debug);

	/*创建设备节点*/
	device_create(hian_class, NULL, MKDEV(numdev_major, numdev_minor), NULL, DEVICE_NAME"%d",0);
	
	#if NO_COMPILE
	debug->v = ioremap(EXYNOS4_PA_ADC, SZ_4K);
	if(!debug->v)
	{
		kfree(debug->v);
		return -ENOMEM;
	}
	printk(KERN_EMERG "no compile is fail!\n");
	#endif
	spin_lock_init(&debug->lock);

	#if NO_COMPILE
	debug->clk = clk_get(NULL, "adc");
	clk_enable(debug->clk);
	#endif	

	init_waitqueue_head(&debug->rq);

	#if NO_COMPILE
	//申请中断
	debug->irq = IRQ_ADC;
	ret = request_irq(debug->irq, debug_isr, 0, "hian_debug_irq", debug);    //第一个参数为中断线，在irqs.h中可以查到，第三个参数触发方式，0为不触发
	if(ret < 0)
	{
		clk_disable(debug->clk);
		clk_put(debug->clk);
		iounmap(debug->v);
		
		return -EFAULT;
	}
	#endif
	//相对时间
	#if 0
	ktime_t t = ktime_set(2, 2000000); //2s + 2ms
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer.function = timer_hander;
	hrtimer_start(&timer, t, HRTIMER_MODE_REL);
	#endif

	//绝对时间
	ktime_t t = ktime_set(1483617149 + 120, 0); //1483617149为RTC获取的时间
	hrtimer_init(&timer, CLOCK_REALTIME, HRTIMER_MODE_ABS);
	timer.function = timer_hander;
	hrtimer_start(&timer, t, HRTIMER_MODE_ABS);

	ret = debug_init(debug);
	if(ret)
	{
		printk(KERN_EMERG "debug init failed!\n");
	}	

	printk(KERN_EMERG "scdev_init!\n");
	/*打印信息，KERN_EMERG表示紧急信息*/
	return 0;

fail:
	/*注销设备号*/
	unregister_chrdev_region(MKDEV(numdev_major, numdev_minor), DEVICE_MINOR_NUM);
	printk(KERN_EMERG "kmalloc is fail!\n");

	return ret;
}

static __exit void scdev_exit(void)
{
	printk(KERN_EMERG "scdev_exit!\n");

	debug_exit(debug);

	#if NO_COMPILE
	free_irq(debug->irq, debug);

	clk_disable(debug->clk);
	clk_put(debug->clk);
	
	iounmap(debug->v);
	#endif

	hrtimer_cancel(&timer);
	/*除去字符设备*/
	cdev_del(&(debug->dev));
	/*摧毁设备节点函数d*/
	device_destroy(hian_class, MKDEV(numdev_major, numdev_minor));

	/*释放设备class*/
	class_destroy(hian_class);

	/*释放内存*/
	kfree(debug);

	unregister_chrdev_region(MKDEV(numdev_major, numdev_minor), DEVICE_MINOR_NUM);
}

module_init(scdev_init);
/*初始化函数*/
module_exit(scdev_exit);
/*卸载函数*/
	
