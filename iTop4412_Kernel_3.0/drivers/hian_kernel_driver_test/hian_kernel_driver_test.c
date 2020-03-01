#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>


static int __init hian_kernel_driver_test_init(void)
{
	int i = 0;
	for(i = 0; i < 10; i++)
	{
		printk(KERN_EMERG "hian kernel driver test!!!\r\n");
	}
	return 0;
}

static void __exit hian_kernel_driver_test_exit(void)
{
	printk(KERN_EMERG "hian kernel driver test exit!!!");
}
module_init(hian_kernel_driver_test_init);
module_init(hian_kernel_driver_test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("hian");
MODULE_VERSION("V1.0");
MODULE_DESCRIPTION("test for driver compile to kernel");


