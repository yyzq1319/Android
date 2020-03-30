#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <asm/atomic.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>

#define DEVICE_NAME "hian_debug"
 
MODULE_LICENSE("GPL");
 
dev_t devno;
int major = 0;
int minor = 0;
int count = 1;
 
#define  KMAX  1024
 
char kbuf[KMAX] = {};
int counter = 0;  //用它记录kbuf中实际存储的字节数量
 
struct cdev *pdev;
 
struct class * pclass;
struct device * pdevice;
 
struct semaphore  sem_r;
struct semaphore  sem_w;
 
wait_queue_head_t  wq;  //创建一个等待队列头
 
int demo_open(struct inode * inodep, struct file * filep)
{
 
	printk("%s,%d\n", __func__, __LINE__);
 
	return 0;
}
 
int demo_release(struct inode *inodep, struct file *filep)
{
 
	printk("%s,%d\n", __func__, __LINE__);
 
	return 0;
}
 
// read(fd, buff, N) --> ... --> demo_read()
ssize_t demo_read(struct file * filep, char __user * buffer, size_t size, loff_t * offlen)
{	
	// 应用程序，读数据时，发现没有资源，那么此时阻塞等代	
	if(counter == 0)
	{
		if(filep->f_flags & O_NONBLOCK)   //设备是阻塞还是非阻塞模式
		{
			return -EAGAIN;
		}
		if(wait_event_interruptible(wq,counter != 0))  //阻塞模式，是否有数据可读
		{
			return -ERESTARTSYS;
		}
	}
 
	down_interruptible(&sem_r);
	if(size > counter)
	{
		size = counter;
	}
	if(copy_to_user(buffer, kbuf, size) != 0)
	{
		printk("Failed to copy_to_user.\n");
		return -EFAULT;
	}
 
	counter = 0;
 
	up(&sem_w);
 
	return size;
}
 
// write(fd, buff, n) --> ... --> demo_write();
ssize_t demo_write(struct file *filep, const char __user *buffer, size_t size, loff_t * offlen)
{
	down_interruptible(&sem_w);
 
	if(size > KMAX)
	{
		return -ENOMEM;
	}
	if(copy_from_user(kbuf, buffer,size) != 0)
	{
		printk("Failed to copy_from_user.\n");
		return -1;
	}
	printk("kbuf:%s\n", kbuf);
	counter = size;
 
	up(&sem_r);
 
	// 唤醒等待队列
	wake_up_interruptible(&wq);   //写入了数据，fifo不为空，可以唤醒读中的等待队列
 
	return size;
}
 
 
struct file_operations  fops = {
	.owner =THIS_MODULE,
	.open = demo_open,
	.release = demo_release,
	.read = demo_read,
	.write = demo_write,
};
 
static int __init demo_init(void)
{
	int ret = 0;
 
	printk("%s,%d\n", __func__, __LINE__);
 
	ret = alloc_chrdev_region(&devno,minor,count, "xxx");
	if(ret)
	{
		printk("Failed to alloc_chrdev_region.\n");
		return ret;
	}
	printk("devno:%d , major:%d  minor:%d\n", devno, MAJOR(devno), MINOR(devno));
 
	pdev = cdev_alloc();
	if(pdev == NULL)
	{
		printk("Failed to cdev_alloc.\n");
		goto err1;
	}
 
	cdev_init(pdev, &fops);
 
	ret = cdev_add(pdev, devno, count);
	if(ret < 0)
	{
	    printk("Failed to cdev_add.");
		goto err2;
	}
 
	pclass = class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(pclass))
	{
		printk("Failed to class_create.\n");
		ret = PTR_ERR(pclass);
		goto err3;
	}
 
	pdevice = device_create(pclass, NULL, devno, NULL, DEVICE_NAME);
	if(IS_ERR(pdevice))
	{
		printk("Failed to device_create.\n");
		ret = PTR_ERR(pdevice);
		goto err4;
	}
 
	sema_init(&sem_r, 0);
	sema_init(&sem_w, 1);
 
	// 初始化等待队列
	init_waitqueue_head(&wq);
 
	return 0;
err4:
	class_destroy(pclass);
err3:
	cdev_del(pdev);
err2:
	kfree(pdev);
err1:
	unregister_chrdev_region(devno, count);
	return ret;
}
 
static void __exit demo_exit(void)
{
	printk("%s,%d\n", __func__, __LINE__);
 
	device_destroy(pclass, devno);
	class_destroy(pclass);
	cdev_del(pdev);
	kfree(pdev);
	unregister_chrdev_region(devno, count);
 
}
 
 
module_init(demo_init);
module_exit(demo_exit);
