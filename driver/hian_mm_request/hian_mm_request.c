#include <linux/module.h>
#include <linux/init.h>
#include <asm/atomic.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>

#define err(msg) printk(KERN_ALERT "%s %s %d %s\n",__FILE__,__FUNCTION__,__LINE__,msg)
#define MEM_LEN  128

//页内存申请
static int memory_page_req(void)
{
	struct page *page;
	void *address;
	
	char data[] = "page memory request test";

	
	page = alloc_pages(GFP_KERNEL,3);//申请2的3次方及8页的内存，GFP_KERNEL在申请内存中允许睡眠，一般在进程中使用，GFP_ATOMIC不允许睡眠，一般在中断中使用
	if(NULL == page)
	{
		err("alloc_pages error");
		return EFAULT;
	}
	
	address = page_address(page);
	if(NULL == address)
	{
		err("alloc_pages error");
		goto err;
	}
	
	memcpy(address,data,strlen(data));
	printk(KERN_ALERT "%s\n",(char *)address);
	
	free_pages((unsigned long)address,0);
	
	return 0;
	
err:
	free_pages((unsigned long)address,0);

	return ENOMEM;
}

//kmalloc内存申请
static int kmalloc_memory_request(void)
{
	void *address;
	char data[] = "kmalloc memory request test";
	
	address = kmalloc(MEM_LEN,GFP_KERNEL);
	if(NULL == address)
	{
		err("kmalloc error");
		goto err;
	}
	 
	memcpy(address,data,strlen(data));
	printk(KERN_ALERT "%s\n",(char *)address);

	kfree(address);

err:
	kfree(address);

	return ENOMEM;
}

//kzalloc内存申请
static int kzalloc_memory_request(void)
{
	char *address;
	char data[] = "kzalloc_memory request test";
	
	address = kzalloc(MEM_LEN,GFP_KERNEL);//申请并清零
	if(NULL == address)
	{
		err("kzalloc error");
		goto err;
	}
	 
	memcpy(address,data,strlen(data));
	printk(KERN_ALERT "%s\n",(char *)address);

	kfree(address);

err:
	kfree(address);

	return ENOMEM;
}

//vmalloc内存申请
static int vmalloc_memory_request(void)
{
	char *address;
	char data[] = "vmalloc memory request test";
	
	address = vmalloc(MEM_LEN);//申请虚拟的空间，物理地址可以不连续
	if(NULL == address)
	{
		err("vmalloc error");
		goto err;
	}
	 
	memcpy(address,data,strlen(data));
	printk(KERN_ALERT "%s\n",(char *)address);

	vfree(address);

err:
	vfree(address);

	return -ENOMEM;
}

//vzalloc内存申请
static int vzalloc_memory_request(void)
{
	char *address;
	char data[] = "vzalloc memory request test";
	
	address = vzalloc(MEM_LEN);//申请并清零
	if(NULL == address)
	{
		err("vzalloc error");
		goto err;
	}
	 
	memcpy(address,data,strlen(data));
	printk(KERN_ALERT "%s\n",(char *)address);

	vfree(address);

err:
	vfree(address);

	return -ENOMEM;
}

//dma内存申请
static int dma_memory_request(void)
{
	char *vaddress;
	dma_addr_t paddr; //dma物理地址，dma总线使用

	vaddress = dma_alloc_coherent(NULL, 1024, &paddr, GFP_KERNEL);
	if(NULL == vaddress)
	{
		err("dma memory request error");
		goto err;
	}
	 
	dma_free_coherent(NULL, 1024, vaddress, paddr);

	return 0;

err:
	vfree(vaddress);

	return -ENOMEM;
}

//申请专门的缓存
struct nrf_st {
	int no;
	int light;
	char msg[16];
};

//ctor在分派缓存中的每一个对象时都会调用，并且把分派的函数的指针传给这个函数
void ctor(void *data)
{

}

static int cache_request(void)
{
	struct kmem_cache *kmem;
	struct nrf_st *nrf;
	
	kmem = kmem_cache_create("hian_cache", sizeof(struct nrf_st), 0, SLAB_HWCACHE_ALIGN, ctor);	
	
	//获取缓存工作对象
	nrf = kmem_cache_alloc(kmem, GFP_KERNEL);
	
	//释放对象
	kmem_cache_free(kmem,nrf);
	kmem_cache_destroy(kmem);
	
	return 0;
}
