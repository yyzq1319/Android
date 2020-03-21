#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/proc_fs.h>  
#include <linux/jiffies.h>  
#include <asm/uaccess.h>  
#include <linux/slab.h>  
#include <linux/fs.h>
#include <linux/seq_file.h>

#define MODULE_VERS "1.0"  
#define PROCFS_DIR_NAME "hian_procfs"
#define PROCFS_FILE_NAME "hian_frocfs_seq_file"
#define PROCFS_MAX_SIZE 100

struct proc_dir_entry *hian_procfs_dir;
struct proc_dir_entry *hian_procfs_seq_file; 		

static void *hian_seq_start(struct seq_file *s, loff_t *pos)
{
	loff_t *spos;
	
	spos = kzalloc(sizeof(loff_t), GFP_KERNEL);
	if(!spos)
	{
		return NULL;
	}
	
	if(*pos > PROCFS_MAX_SIZE)
	{
		return NULL;
	}

	*spos = *pos + 50;

	printk(KERN_INFO "*pos = %lld\r\n", *pos); 
    
	return spos;
}

static void *hian_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	loff_t *spos = (loff_t *)v;
	printk(KERN_INFO "*spos = %lld\r\n", *spos); 
	
	*pos = ++(*spos);
	if(*pos > PROCFS_MAX_SIZE)
	{
		return NULL;
	}
	
	return spos;
}

static void hian_seq_stop(struct seq_file *s, void *v)
{
	printk("hian_seq_Stop!\n");
	kfree(v);
}

static int hian_seq_show(struct seq_file *s, void *v)
{
	loff_t *spos = (loff_t *)v;
	seq_printf(s, "%lld\n", *spos);
	
	return 0;
}

static const struct seq_operations hian_seq_ops = {
	.start = hian_seq_start,
	.next = hian_seq_next,
	.stop = hian_seq_stop,
	.show = hian_seq_show
};

static int hian_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "/proc/%s open......\n", PROCFS_FILE_NAME); 
	
	return seq_open(file, &hian_seq_ops);
}

static const struct file_operations hian_file_ops = {
	.owner = THIS_MODULE,
	.open = hian_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
	
};

static int __init hian_procfs_init(void)
{
	hian_procfs_dir = proc_mkdir(PROCFS_DIR_NAME, NULL);
    if(NULL == hian_procfs_dir)
    {
		remove_proc_entry(PROCFS_DIR_NAME, NULL);
		printk(KERN_ALERT "Error: could not create proc dir /proc/%s\n", PROCFS_DIR_NAME);
	
		return -ENOMEM;
    }
	
	hian_procfs_seq_file = create_proc_entry(PROCFS_FILE_NAME, 0666, hian_procfs_dir);
    if(NULL == hian_procfs_seq_file)
    {
		remove_proc_entry(PROCFS_FILE_NAME, hian_procfs_dir);
		remove_proc_entry(PROCFS_DIR_NAME, NULL);
        printk(KERN_ALERT "Error: could not create proc file /proc/%s\n", PROCFS_DIR_NAME);

		return -ENOMEM;
    }
	
	hian_procfs_seq_file->proc_fops = &hian_file_ops;
 
	printk(KERN_INFO "/proc/%s created\n", PROCFS_FILE_NAME); 
    
	return 0;	
}

//static int __exit hian_procfs_exit(void)
static void  hian_procfs_exit(void)
{
	remove_proc_entry(PROCFS_FILE_NAME, hian_procfs_dir);
	remove_proc_entry(PROCFS_DIR_NAME, NULL);
}


module_init(hian_procfs_init);  
module_exit(hian_procfs_exit);  

MODULE_AUTHOR("hian");  
MODULE_DESCRIPTION("procfs examples");  
MODULE_LICENSE("GPL");
