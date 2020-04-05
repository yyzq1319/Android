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
#include "hian_adc.h"

int numdev_major = DEV_MAJOR;
int numdev_minor = DEV_MINOR;

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

struct class *hian_class;
struct adc_st *adc;

static int adc_init(struct adc_st *adc)
{
	unsigned int tmp;

	adc->adccon = (1 << 16) | (1 << 14) | (24 << 6); //ADC产生中断
	iowrite32(adc->adccon, adc->v + ADCCON);
	adc->adcdly = 0xffff;
	iowrite32(adc->adcdly, adc->v + ADCDLY);
	
	//中断配置
	tmp = ioread32(adc->v_iesr + IESR2);
	tmp |= 1 << 19;
	iowrite32(tmp, adc->v_iesr + IESR2);

	//通道选择	
	iowrite32(0, adc->v + ADCMUX);
	printk(KERN_EMERG "adc hw init!\r\n");
	
	return 0;
}

static int adc_exit(struct adc_st *adc)
{
	printk(KERN_EMERG "adc exit\n");
	
	return 0;
}

static void  adc_enable(struct adc_st *adc)
{
	adc->adccon = ioread32(adc->v + ADCCON);
	adc->adccon |= 1;
	iowrite32(adc->adccon, adc->v + ADCCON);
	printk(KERN_EMERG "adc enable!\r\n");
}

static void adc_disable(struct adc_st *adc)
{
	adc->adccon = ioread32(adc->v + ADCCON);
	adc->adccon &= ~1;
	iowrite32(adc->adccon, adc->v + ADCCON);
	printk(KERN_EMERG "adc disable!\r\n");
}

static int get_adc(struct adc_st *adc)
{
	return ioread32(adc->v + ADCDAT) & 0xfff;
}

void adc_clrInt(struct adc_st *adc)
{
	iowrite32(1, adc->v + ADCCLEARINT);
}

irqreturn_t adc_isr(int irq, void *data)
{
	struct adc_st *adc;

	adc = data;

	printk(KERN_EMERG "adc irq isr......\r\n");
	adc->value = get_adc(adc);
	adc->flag = HAVE_DATA;	

	wake_up(&(adc->rq));

	adc_clrInt(adc);

	return IRQ_HANDLED;
}

static long hian_adc_ioctl( struct file *file, unsigned int cmd, unsigned long num)
{
	struct adc_st *adc;

	adc = file->private_data;	
	switch(cmd)
	{
		case 0:
			adc_enable(adc);
			break;
		case 1:
			adc_disable(adc);
			break;
		default:
			break;
	}

	return 0;
}

static int hian_adc_open(struct inode *inode, struct file *file)
{
	struct adc_st *adc;

	adc = container_of(inode->i_cdev, struct adc_st, dev);
	file->private_data = adc;

	spin_lock(&adc->lock);
	if(adc->lock_count)
	{
		spin_unlock(&adc->lock);
		return -EBUSY;
	}
	adc->lock_count = 1;
	spin_unlock(&adc->lock);

	printk(KERN_EMERG "hian_adc open\n");
	return 0;
}

static int hian_adc_release(struct inode *inode, struct file *file)
{
	struct adc_st *adc;
	
	adc = file->private_data;
	spin_lock(&adc->lock);
	if(!(adc->lock_count))
	{
		spin_unlock(&adc->lock);
		return -ENODEV;
	}
	adc->lock_count = 0;
	spin_unlock(&adc->lock);

	printk(KERN_EMERG "hian_adc release\n");
	return 0;
}
ssize_t hian_adc_read(struct file *file, char __user *buffer, size_t count, loff_t *off)
{
	struct adc_st *adc;
	struct adc_value_st avs;
	int ret;

	adc = file->private_data;

	if(count != sizeof(avs))
	{
		printk(KERN_EMERG "read is fail:count =  %d\n", count);
		return -EINVAL;
	}	

	while(adc->flag == NO_DATA)
	{
		if(file->f_flags & O_NONBLOCK) //非阻塞访问
		{
			printk(KERN_EMERG "read is fail:O_NONBLOCK!\r\n");
			return -EAGAIN;
		}

		//阻塞访问	
		wait_event_interruptible(adc->rq, adc->flag == HAVE_DATA);
	}

	avs.value = adc->value;
	ret = copy_to_user(buffer, &avs.value, sizeof(avs));
	if(ret)
	{
		printk(KERN_EMERG "read is fail:copy_to_user!\r\n");
		return -EFAULT;
	}
	
	adc->flag = NO_DATA;
	
	return count;

}

static int hian_adc_write(struct file *file, const char *buffer, size_t count, loff_t *off)
{
	return count;
}

unsigned int hian_adc_poll(struct file *file, struct poll_table_struct *table)
{
	struct adc_st *adc;
	unsigned int mask = 0;

	adc = file->private_data;
	poll_wait(file, &adc->rq, table);

	if(adc->flag == HAVE_DATA)
	{
		mask |= POLLIN;
	}
	
	return mask;
}

static struct file_operations hian_adc_ops = {
	.owner = THIS_MODULE,
	.open = hian_adc_open,
	.read = hian_adc_read,
	.write = hian_adc_write,
	.release = hian_adc_release,
	.unlocked_ioctl = hian_adc_ioctl,
	.poll = hian_adc_poll,
};


static void reg_init_cdev(struct adc_st *adc)
{
	int err;
	 adc->no = MKDEV(numdev_major, numdev_minor);//获取设备号

	/*数据初始化*/
	cdev_init(&adc->dev, &hian_adc_ops);
	adc->dev.owner = THIS_MODULE;
	adc->dev.ops = &hian_adc_ops;

	/*注册驱动到系统*/
	err = cdev_add(&adc->dev, adc->no, 1);
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

	adc = kzalloc(sizeof(struct adc_st), GFP_KERNEL);
	if(!adc)
	{
		ret = -ENOMEM;
		goto fail;
	}

	/*动态注册设备号*/
	ret = alloc_chrdev_region(&adc->no, 1, DEVICE_MINOR_NUM, DEVICE_NAME);

	/*获得主设备号*/
	numdev_major = MAJOR(adc->no);
	printk(KERN_EMERG "adev_region req %d !\n", numdev_major);
	if(ret<0)
	{
		printk(KERN_EMERG "register_chrdev_region req %d is failed!\n", numdev_major);	
	}

	hian_class = class_create(THIS_MODULE, DEVICE_NAME);

	/*设备初始化*/
	adc->flag = NO_DATA;

	/*设备注册到系统*/
	reg_init_cdev(adc);

	/*创建设备节点*/
	device_create(hian_class, NULL, MKDEV(numdev_major, numdev_minor), NULL, DEVICE_NAME"%d",0);

	adc->v = ioremap(EXYNOS4_PA_ADC, SZ_4K);
	if(!adc->v)
	{
		kfree(adc->v);
		return -ENOMEM;
	}	

	adc->v_iesr = ioremap(EXYNOS4_PA_IESR, SZ_4K);//共用中断屏蔽
	if(!adc->v_iesr)
	{
		kfree(adc->v_iesr);
		kfree(adc->v);
		return -ENOMEM;
	}	
	
	spin_lock_init(&adc->lock);

	adc->clk = clk_get(NULL, "adc");
	clk_enable(adc->clk);
	adc->clkphy = clk_get(NULL, "adcphy");
	clk_enable(adc->clkphy);
	adc->fsys_clk = clk_get(NULL, "fsys_adc");
	clk_enable(adc->clk);
	clk_enable(adc->clkphy);
	clk_enable(adc->fsys_clk);

	init_waitqueue_head(&adc->rq);

	//申请ADC中断
	adc->irq = IRQ_ADC;
	ret = request_irq(adc->irq, adc_isr, 0, "hian_adc_irq", adc);    //第一个参数为中断线，在irqs.h中可以查到，第三个参数触发方式，0为不触发
	if(ret < 0)
	{
		clk_disable(adc->clk);
		clk_put(adc->clk);
		clk_disable(adc->clkphy);
		clk_put(adc->clk);
		clk_disable(adc->fsys_clk);
		clk_put(adc->fsys_clk);
		iounmap(adc->v);
		iounmap(adc->v_iesr);
		
		return -EFAULT;
	}

	ret = adc_init(adc);
	if(ret)
	{
		printk(KERN_EMERG "adc init failed!\n");
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

	adc_exit(adc);

	free_irq(adc->irq, adc);

	clk_disable(adc->clk);
	clk_put(adc->clk);
	clk_disable(adc->clkphy);
	clk_put(adc->clkphy);
	clk_disable(adc->fsys_clk);
	clk_put(adc->fsys_clk);
	
	iounmap(adc->v_iesr);
	iounmap(adc->v);

	/*除去字符设备*/
	cdev_del(&(adc->dev));
	/*摧毁设备节点函数d*/
	device_destroy(hian_class, MKDEV(numdev_major, numdev_minor));

	/*释放设备class*/
	class_destroy(hian_class);

	/*释放内存*/
	kfree(adc);

	unregister_chrdev_region(MKDEV(numdev_major, numdev_minor), DEVICE_MINOR_NUM);
}

module_init(scdev_init);
/*初始化函数*/
module_exit(scdev_exit);
/*卸载函数*/
