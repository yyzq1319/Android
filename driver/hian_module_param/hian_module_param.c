#include <linux/init.h>
#include <linux/module.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
/*注册设备节点的文件结构体*/
#include <linux/fs.h>
/*定义module_param_array的头文件*/
#include <linux/moduleparam.h>
/*定义module_param module_param_array函数中perm的头文件*/
#include <linux/stat.h>

#define DRIVER_NAME "hian_module_param"  
#define DEVICE_NAME "hian_module_param"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("HIAN");

static int module_arg1,module_arg2;
static int int_array[50];
static int int_num;

module_param(module_arg1, int, S_IRUSR);
module_param(module_arg2, int, S_IRUSR);
module_param_array(int_array, int, &int_num, S_IRUSR);

static long hian_module_param_ioctl( struct file *files, unsigned int cmd, unsigned long num)
{
	return 0;
}

static int hian_module_param_release(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_module_param release\n");
	return 0;
}

static int hian_module_param_open(struct inode *inode, struct file *file)
{
	printk(KERN_EMERG "hian_module_param open\n");
	return nonseekable_open(inode,file);
}

static struct file_operations hian_module_param_ops = {
	.owner = THIS_MODULE,
	.open = hian_module_param_open,
	.release = hian_module_param_release,
	.unlocked_ioctl = hian_module_param_ioctl,
};

static  struct miscdevice hian_module_param_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &hian_module_param_ops,
};

static int hian_module_param_probe(struct platform_device *pdv){
	int ret;
	unsigned int num = 0;
	
	printk(KERN_EMERG "\tinitialized\n");
	
	misc_register(&hian_module_param_dev);
	
	return 0;
}

static int hian_module_param_remove(struct platform_device *pdv){
	
	printk(KERN_EMERG "\thian module_param remove\n");
	misc_deregister(&hian_module_param_dev);
	return 0;
}

static void hian_module_param_shutdown(struct platform_device *pdv){
	
	;
}

static int hian_module_param_suspend(struct platform_device *pdv,pm_message_t pmt){
	
	return 0;
}

static int hian_module_param_resume(struct platform_device *pdv){
	
	return 0;
}

struct platform_driver hian_module_param_driver = {
	.probe = hian_module_param_probe,
	.remove = hian_module_param_remove,
	.shutdown = hian_module_param_shutdown,
	.suspend = hian_module_param_suspend,
	.resume = hian_module_param_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	}
};

static int hian_module_param_init(void)
{
	int DriverState;
	int i;

	printk(KERN_EMERG "module_arg1 is %d!\n",module_arg1);		
	printk(KERN_EMERG "module_arg2 is %d!\n",module_arg2);		
	for(i=0;i<int_num;i++)
	{
		printk(KERN_EMERG "int_array[%d] is %d!\n",i,int_array[i]);		
	}		

	printk(KERN_EMERG "HIAN MODULE PARAM enter!\n");
	DriverState = platform_driver_register(&hian_module_param_driver);
	
	printk(KERN_EMERG "\tDriverState is %d\n",DriverState);
	return 0;
}


static void hian_module_param_exit(void)
{
	printk(KERN_EMERG "HIAN MODULE PARAM exit!\n");
	
	platform_driver_unregister(&hian_module_param_driver);	
}

module_init(hian_module_param_init);
module_exit(hian_module_param_exit);
