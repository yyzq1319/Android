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
#include <linux/semaphore.h>
#include "hian_debug.h"

int numdev_major = DEV_MAJOR;
int numdev_minor = DEV_MINOR;

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

struct class *hian_class;
struct debug_st *debug;
struct semaphore sem1;
struct semaphore sem2;

int data = 0;
int num[2][5] = {
	{0,2,4,6,8},
	{1,3,5,7,9}		
};

int thread_one(void *p)
{
	int *num = (int *)p;
	int i;
	
	down(&sem1);
	for(i=0; i<5; i++)
	{
	//	down(&sem1);
		data++;
		printk(KERN_EMERG "thread one:%d\n", data);
	//	up(&sem1);
	}
	up(&sem1);
	
	return 0;	
}

int thread_two(void *p)
{
	int *num = (int *)p;
	int i;

	down(&sem1);
	for(i=0; i<5; i++)
	{
		//down(&sem2);
		data--;
		printk(KERN_EMERG "thread two:%d\n", data);
	
		//up(&sem2);
	}
	up(&sem1);

	return 0;
}

static long hian_debug_ioctl( struct file *fp, unsigned int cmd, unsigned long num)
{
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
	return count;

}

static int hian_debug_write(struct file *fp, const char *buffer, size_t count, loff_t *off)
{
	return count;
}

unsigned int hian_debug_poll(struct file *fp, struct poll_table_struct *table)
{
	unsigned int mask = 0;

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


	/*设备注册到系统*/
	reg_init_cdev(debug);

	/*创建设备节点*/
	device_create(hian_class, NULL, MKDEV(numdev_major, numdev_minor), NULL, DEVICE_NAME"%d",0);

	sema_init(&sem1, 1); //初始化信号量1,使信号量1最初可被获取
	sema_init(&sem2, 1);//初始化信号量2,使信号量只有被释放后才可被获取
	kernel_thread(thread_one, num[0], CLONE_KERNEL);
	kernel_thread(thread_two, num[1], CLONE_KERNEL);
		
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
