/*模块头文件*/
#include <linux/init.h>
#include <linux/module.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
/*注册设备节点的文件结构体*/
#include <linux/fs.h>
/*原子操作头文件*/
#include <asm/atomic.h>
#include <asm/types.h>

#define DRIVER_NAME "hian_atomic_op"
#define DEVICE_NAME "hian_atomic_op"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

static atomic_t iAtomicValue = ATOMIC_INIT(0);//atomic int value
unsigned long int bAtomicValue = 0;

static int hian_atomic_op_open(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_atomic_op open in!\n");	
	
	//int value read
	if(atomic_read(&iAtomicValue))
	{
		printk(KERN_EMERG "int AtomicValue read fail!\n");	
		return -EBUSY;
	}
	else
	{
		printk(KERN_EMERG "iAtomicValue = %d!\n",iAtomicValue);	
	}
	
	//bit value read
	if(test_bit(0,&bAtomicValue)!=0)
	{
		printk(KERN_EMERG "bit AtomicValue read fail!\n");	
		return -EBUSY;
	}
	else
	{
		printk(KERN_EMERG "bAtomicValue = %d!\n",bAtomicValue);	
	}
	
	atomic_set(&iAtomicValue,0x5a);
	set_bit(0,&bAtomicValue);
	atomic_read(&iAtomicValue);
	printk(KERN_EMERG "iAtomicValue = %d!\n",iAtomicValue);	
	test_bit(0,&bAtomicValue);
	printk(KERN_EMERG "bAtomicValue = %d!\n",bAtomicValue);	

	printk(KERN_EMERG "hian_atomic_op open success!\n");	
	return 0;
}

static int hian_atomic_op_release(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_atomic_op release\n");
	atomic_set(&iAtomicValue,0);
	clear_bit(0,&bAtomicValue);
	
	return 0;
}

static struct file_operations hian_atomic_op_ops = {
	.owner = THIS_MODULE,
	.open = hian_atomic_op_open,
	.release = hian_atomic_op_release,
};


static  struct miscdevice hian_atomic_op_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &hian_atomic_op_ops,
};


static int hian_atomic_op_probe(struct platform_device *pdv)
{
	
	printk(KERN_EMERG "\thian_atomic_op initialized\n");
	misc_register(&hian_atomic_op_dev);
	
	return 0;
}

static int hian_atomic_op_remove(struct platform_device *pdv)
{
	
	printk(KERN_EMERG "\thian_atomic_op remove\n");
	misc_deregister(&hian_atomic_op_dev);
	return 0;
}

struct platform_driver hian_atomic_op_driver = {
	.probe = hian_atomic_op_probe,
	.remove = hian_atomic_op_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	}
};


static int hian_atomic_op_init(void)
{
	int DriverState;
	
	printk(KERN_EMERG "hian_atomic_op enter!\n");
	DriverState = platform_driver_register(&hian_atomic_op_driver);
	
	printk(KERN_EMERG "\thian_atomic_op DriverState is %d\n",DriverState);
	return 0;
}


static void hian_atomic_op_exit(void)
{
	printk(KERN_EMERG "hian_atomic_op exit!\n");
	
	platform_driver_unregister(&hian_atomic_op_driver);	
}

module_init(hian_atomic_op_init);
module_exit(hian_atomic_op_exit);
