#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/proc_fs.h>  
#include <linux/jiffies.h>  
#include <asm/uaccess.h>  
#include <linux/slab.h>  


#define MODULE_VERS "1.0"  
#define PROCFS_NAME "hian_procfs"

#define PROCFS_MAX_SIZE 1024

struct hian_dev_st {
	char buffer[PROCFS_MAX_SIZE];
	int len;
	struct proc_dir_entry *hian_procfs_dir;
	struct proc_dir_entry *hian_procfs_file; 		
};

struct hian_dev_st *hian_dev;

//proc read
static int procfile_read(char *buffer, char **start, off_t offset, int count, int *eof, void* data)
{
	struct hian_dev_st *d;
	
	d = data;
	if(offset > (d->len - 1))//保证能读到最后一个数据
	{
		*eof = 1;
		return 0;
	}

	count = min(count, (d->len - (int)offset));
	sprintf(buffer, (d->buffer + offset), count);
	*start = (void *)count;

	return count;
}

//proc write
static int procfile_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct hian_dev_st *d;
	int ret = 0;

	d = data;
	count = min(PROCFS_MAX_SIZE, (int)count);
	ret = copy_from_user(d->buffer, buffer, count);
	if(ret)
	{
		return -ENOMEM;
	}	
	
	d->len = count;
	
	return count;
}

static int __init hian_procfs_init(void)
{

	hian_dev = kzalloc(sizeof(*hian_dev), GFP_KERNEL);
	if(NULL == hian_dev)
	{
		kfree(hian_dev);
		return -ENOMEM;
	}
    
	hian_dev->hian_procfs_dir = proc_mkdir(PROCFS_NAME, NULL);
    if(NULL == hian_dev->hian_procfs_dir)
    {
		remove_proc_entry(PROCFS_NAME, NULL);
		printk(KERN_ALERT "Error: could not create proc dir /proc/%s\n", PROCFS_NAME);
	
		return -ENOMEM;
    }
	
	hian_dev->hian_procfs_file = create_proc_entry("hian_procfs_file", 0644, hian_dev->hian_procfs_dir);
    if(NULL == hian_dev->hian_procfs_file)
    {
		remove_proc_entry("hian_procfs_file", hian_dev->hian_procfs_dir);
        printk(KERN_ALERT "Error: could not create proc file /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
    }
	 
	hian_dev->hian_procfs_file->read_proc = procfile_read; 
	hian_dev->hian_procfs_file->write_proc = procfile_write; 
	hian_dev->hian_procfs_file->data = hian_dev;
	
	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME); 
    
	return 0;	
}

static int __exit hian_procfs_exit(void)
{
    
	remove_proc_entry("hian_procfs_file", hian_dev->hian_procfs_dir);
	remove_proc_entry("hian_procfs", NULL);
	kfree(hian_dev);
    
	return 0;
}


module_init(hian_procfs_init);  
module_exit(hian_procfs_exit);  

MODULE_AUTHOR("hian");  
MODULE_DESCRIPTION("procfs examples");  
MODULE_LICENSE("GPL");
