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
#define PROCFS_FILE_NAME "hian_frocfs_seq_file_list"
#define PROCFS_MAX_SIZE 100

struct proc_dir_entry *hian_procfs_dir;
struct proc_dir_entry *hian_procfs_seq_file_list; 		

static struct mutex lock;
static struct list_head head;

struct hian_data {
	char value[PROCFS_MAX_SIZE];
	struct list_head list;
};

static void *hian_seq_start(struct seq_file *s, loff_t *pos)
{
	mutex_lock(&lock);
	return seq_list_start(&head, *pos);
}

static void *hian_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	return seq_list_next(v, &head, pos);
}

static void hian_seq_stop(struct seq_file *s, void *v)
{
	mutex_unlock(&lock);
}

static int hian_seq_show(struct seq_file *s, void *v)
{
	struct hian_data *data;
	
	data = list_entry(v, struct hian_data, list);

	seq_printf(s, "data:%s\n", data->value);
	
	return 0;
}

static const struct seq_operations hian_seq_ops = {
	.start = hian_seq_start,
	.next = hian_seq_next,
	.stop = hian_seq_stop,
	.show = hian_seq_show
};

static void hian_add_node(void)
{
	struct hian_data *data;
	
	mutex_lock(&lock);
	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if(data)
	{
		list_add(&data->list, &head);
	}
	
	mutex_unlock(&lock);
}

static ssize_t hian_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	hian_add_node();
	return count;
}

static void hian_delete_node(struct list_head *head)
{
	struct hian_data *data;
	struct list_head *tmp;

	//list_for_each(tmp, &head)
	while(!list_empty(head))
	{
		data = list_entry(tmp, struct hian_data, list);
		list_del(&data->list);
		kfree(data);
	}
}

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
	.release = seq_release,
	.write = hian_write
	
};

static int __init hian_procfs_init(void)
{
	mutex_init(&lock);
	INIT_LIST_HEAD(&head);

	hian_procfs_dir = proc_mkdir(PROCFS_DIR_NAME, NULL);
    if(NULL == hian_procfs_dir)
    {
		remove_proc_entry(PROCFS_DIR_NAME, NULL);
		printk(KERN_ALERT "Error: could not create proc dir /proc/%s\n", PROCFS_DIR_NAME);
	
		return -ENOMEM;
    }
	
	hian_procfs_seq_file_list = create_proc_entry(PROCFS_FILE_NAME, 0666, hian_procfs_dir);
    if(NULL == hian_procfs_seq_file_list)
    {
		remove_proc_entry(PROCFS_FILE_NAME, hian_procfs_dir);
		remove_proc_entry(PROCFS_DIR_NAME, NULL);
        printk(KERN_ALERT "Error: could not create proc file /proc/%s\n", PROCFS_DIR_NAME);

		return -ENOMEM;
    }
	
	hian_procfs_seq_file_list->proc_fops = &hian_file_ops;
 
	printk(KERN_INFO "/proc/%s created\n", PROCFS_FILE_NAME); 
    
	return 0;	
}

static void  hian_procfs_exit(void)
{
	hian_delete_node(&head);
	remove_proc_entry(PROCFS_FILE_NAME, hian_procfs_dir);
	remove_proc_entry(PROCFS_DIR_NAME, NULL);
}


module_init(hian_procfs_init);  
module_exit(hian_procfs_exit);  

MODULE_AUTHOR("hian");  
MODULE_DESCRIPTION("procfs examples");  
MODULE_LICENSE("GPL");
