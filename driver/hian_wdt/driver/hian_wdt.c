/*包含初始化宏定义的头文件,代码中的module_init和module_exit在此文件中*/
#include <linux/init.h>
/*包含初始化加载模块的头文件,代码中的MODULE_LICENSE在此头文件中*/
#include <linux/module.h>
/*定义module_param module_param_array的头文件*/
#include <linux/moduleparam.h>
/*定义module_param module_param_array中perm的头文件*/
#include <linux/stat.h>
/*三个字符设备函数*/
#include <linux/fs.h>
/*MKDEV转换设备号数据类型的宏定义*/
#include <linux/kdev_t.h>
/*定义字符设备的结构体*/
#include <linux/cdev.h>
/*分配内存空间函数头文件*/
#include <linux/slab.h>
/*包含函数device_create 结构体class等头文件*/
#include <linux/device.h>
/*自定义头文件*/
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include "hian_wdt.h"

int numdev_major = DEV_MAJOR;
int numdev_minor = DEV_MINOR;

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

struct class *hian_class;
struct wdt_st *wdt;
dev_t dev_no;

static int wdt_init(struct wdt_st *wdt)
{
	wdt->wdtcon = (99 << 8) | (2 << 3) | 1;

	wdt->wdtcnt = 15625 * 3;
	
	iowrite32(wdt->wdtcon, wdt->v + WTCON);
	iowrite32(wdt->wdtcnt, wdt->v + WTCNT);
	
	return 0;
}

static int wdt_exit(struct wdt_st *wdt)
{
	printk(KERN_EMERG "wdt exit\n");
	
	return 0;
}

static void  wdt_enable(struct wdt_st *wdt)
{
	wdt->wdtcon = ioread32(wdt->v + WTCON);
	wdt->wdtcon |= 1 << 5;
	iowrite32(wdt->wdtcon, wdt->v + WTCON);
}

static void wdt_disable(struct wdt_st *wdt)
{
	wdt->wdtcon = ioread32(wdt->v + WTCON);
	wdt->wdtcon &= ~(1 << 5);
	iowrite32(wdt->wdtcon, wdt->v + WTCON);
}

static void feed_wdt(struct wdt_st *wdt, int ms)
{
	iowrite32(ms, wdt->v + WTCNT);
}

static long hian_wdt_ioctl( struct file *file, unsigned int cmd, unsigned long num)
{
	struct wdt_st *wdt;

	wdt = file->private_data;	
	switch(cmd)
	{
		case 0:
			wdt_enable(wdt);
			break;
		case 1:
			wdt_disable(wdt);
			break;
		default:
			break;
	}

	return 0;
}

static int hian_wdt_open(struct inode *inode, struct file *file)
{
	struct wdt_st *wdt;

	wdt = container_of(inode->i_cdev, struct wdt_st, dev);
	file->private_data = wdt;

	spin_lock(&wdt->lock);
	if(wdt->count)
	{
		spin_unlock(&wdt->lock);
		return -EBUSY;
	}
	wdt->count = 1;
	spin_unlock(&wdt->lock);

	printk(KERN_EMERG "hian_wdt open\n");
	return 0;
}

static int hian_wdt_release(struct inode *inode, struct file *file)
{
	struct wdt_st *wdt;
	
	wdt = file->private_data;
	spin_lock(&wdt->lock);
	if(!(wdt->count))
	{
		spin_unlock(&wdt->lock);
		return -ENODEV;
	}
	wdt->count = 0;
	spin_unlock(&wdt->lock);

	printk(KERN_EMERG "hian_wdt release\n");
	return 0;
}
static int hian_wdt_write(struct file *file, const char *buffer, size_t count, loff_t *off)
{
	struct wdt_st *wdt;
	struct wdt_feed_st wfs;
	int ret;
	
	if(count != sizeof(wfs))
	{
		return -EINVAL;
	}

	wdt = file->private_data;

	ret = copy_from_user(&wfs, buffer, sizeof(wfs));
	if(ret)
	{
		return -EFAULT;
	}

	feed_wdt(wdt, wfs.times);	

//	printk(KERN_EMERG "feed wdt time = %d!\n", wfs.times);

	return count;
}
static struct file_operations hian_wdt_ops = {
	.owner = THIS_MODULE,
	.open = hian_wdt_open,
	.write = hian_wdt_write,
	.release = hian_wdt_release,
	.unlocked_ioctl = hian_wdt_ioctl,
};


static void reg_init_cdev(struct wdt_st *wdt)
{
	int err;
	 dev_no = MKDEV(numdev_major, numdev_minor);//获取设备号

	/*数据初始化*/
	cdev_init(&wdt->dev, &hian_wdt_ops);
	wdt->dev.owner = THIS_MODULE;
	wdt->dev.ops = &hian_wdt_ops;

	/*注册驱动到系统*/
	err = cdev_add(&wdt->dev, dev_no, 1);
	if(err)
	{
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

	/*动态注册设备号*/
	ret = alloc_chrdev_region(&dev_no, 1, DEVICE_MINOR_NUM, DEVICE_NAME);

	/*获得主设备号*/
	numdev_major = MAJOR(dev_no);
	printk(KERN_EMERG "adev_region req %d !\n", numdev_major);

	if(ret<0)
	{
		printk(KERN_EMERG "register_chrdev_region req %d is failed!\n", numdev_major);	
	}

	hian_class = class_create(THIS_MODULE, DEVICE_NAME);
	wdt = kzalloc(sizeof(struct wdt_st), GFP_KERNEL);
	if(!wdt)
	{
		ret = -ENOMEM;
		goto fail;
	}

	/*设备初始化*/

	/*设备注册到系统*/
	reg_init_cdev(wdt);

	/*创建设备节点*/
	device_create(hian_class, NULL, MKDEV(numdev_major, numdev_minor), NULL, DEVICE_NAME"%d",0);

	wdt->v = ioremap(EXYNOS4_PA_WDT, SZ_4K);
	if(!wdt->v)
	{
		kfree(wdt->v);
		return -ENOMEM;
	}	
	
	spin_lock_init(&wdt->lock);

	wdt->clk = clk_get(NULL, "watchdog");
	clk_enable(wdt->clk);

	ret = wdt_init(wdt);
	if(ret)
	{
		printk(KERN_EMERG "wdt init failed!\n");
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

	wdt_exit(wdt);

	clk_disable(wdt->clk);
	clk_put(wdt->clk);

	iounmap(wdt->v);

	/*除去字符设备*/
	cdev_del(&(wdt->dev));
	/*摧毁设备节点函数d*/
	device_destroy(hian_class, MKDEV(numdev_major, numdev_minor));

	/*释放设备class*/
	class_destroy(hian_class);

	/*释放内存*/
	kfree(wdt);

	unregister_chrdev_region(MKDEV(numdev_major, numdev_minor), DEVICE_MINOR_NUM);
}

module_init(scdev_init);
/*初始化函数*/
module_exit(scdev_exit);
/*卸载函数*/
