#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/proc_fs.h>  
#include <linux/jiffies.h>  
#include <asm/uaccess.h>  
#include <linux/slab.h>  


#define MODULE_VERS "1.0"  
#define PROCFS_DIR_NAME "hian_procfs"
#define PROCFS_FILE_NAME "hian_procfs_list_file"
#define PROCFS_MAX_SIZE 1024

struct proc_dir_entry *hian_procfs_dir;
struct proc_dir_entry *hian_procfs_list_file; 		

struct head_st{
	int count; //所有节点数据量总和
	struct list_head h;
}head;

//每个节点数据结构
struct item_st{
	int len; //该节点中的数据量
	char *content; //指向节点数据的指针
	struct list_head i;
};

//proc read
static int procfile_read(char *buffer, char **start, off_t offset, int count, int *eof, void* data)
{
	struct list_head *tmp;
	struct item_st *item;
	int all = 0;

	//读越界判断	
	if(offset > (head.count - 1)) 
	{
		*eof = 1;
		return 0;
	}		

	list_for_each(tmp, &(head.h))
	{
		item = list_entry(tmp, struct item_st, i);
		if((item->len + all) > offset)  //前面的节点长度加本节点的长度
		{
			count = min(count, item->len -(int)(offset-all));
			memcpy(buffer, item->content + (offset - all), count);
			break;
		}
		
		all += item->len;
	}
	
	*start  = (void *)count;
	
	return count;
}

//proc write
static int procfile_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct item_st *item;
	int ret = 0;

	item = kzalloc(sizeof(*item) + count, GFP_KERNEL); //结构体内部为指针，非数组，所以加count
	if(NULL == item)
	{
		kfree(item);
		return -ENOMEM;
	}
	
	item->len = count;
	item->content = (void*)(item + 1);
	
	ret = copy_from_user(item->content, buffer, count);
	if(ret)
	{
		printk(KERN_ALERT "Error: copy_from_user /proc/%s\n", PROCFS_DIR_NAME);
	
		kfree(item);
		return -ENOMEM;
	}	
	
	head.count += item->len;
	list_add_tail(&(item->i), &(head.h)); //将新节点插入到一个已知节点的前面
	
	return count;
}

static int __init hian_procfs_init(void)
{
	hian_procfs_dir = proc_mkdir(PROCFS_DIR_NAME, NULL);
    if(NULL == hian_procfs_dir)
    {
		remove_proc_entry(PROCFS_DIR_NAME, NULL);
		printk(KERN_ALERT "Error: could not create proc dir /proc/%s\n", PROCFS_DIR_NAME);
	
		return -ENOMEM;
    }
	
	hian_procfs_list_file = create_proc_entry(PROCFS_FILE_NAME, 0666, hian_procfs_dir);
    if(NULL == hian_procfs_list_file)
    {
		remove_proc_entry(PROCFS_FILE_NAME, hian_procfs_dir);
		remove_proc_entry(PROCFS_DIR_NAME, NULL);
        printk(KERN_ALERT "Error: could not create proc file /proc/%s\n", PROCFS_DIR_NAME);

		return -ENOMEM;
    }
	 
	hian_procfs_list_file->read_proc = procfile_read; 
	hian_procfs_list_file->write_proc = procfile_write; 
	
	INIT_LIST_HEAD(&(head.h));
	
	printk(KERN_INFO "/proc/%s created\n", PROCFS_DIR_NAME); 
    
	return 0;	
}

//static int __exit hian_procfs_exit(void)
static int  hian_procfs_exit(void)
{
    
	remove_proc_entry(PROCFS_FILE_NAME, hian_procfs_dir);
	remove_proc_entry(PROCFS_DIR_NAME, NULL);
    
	return 0;
}


module_init(hian_procfs_init);  
module_exit(hian_procfs_exit);  

MODULE_AUTHOR("hian");  
MODULE_DESCRIPTION("procfs examples");  
MODULE_LICENSE("GPL");
