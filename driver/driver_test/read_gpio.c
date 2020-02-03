#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<unistd.h>

#define GPIO_NUM 2
#define GPIO_CMD 3

static char *hian_gpio_device[]={
	"/dev/hian_gpio_input0",
	"/dev/hian_gpio_input1",
};

int read_gpio(int num, int cmd)
{
	int fd;
   	char *read_gpio;

    /*两个灯，分别为0和1 */
    if(num >= GPIO_NUM)
    {
		printf("gpio num error!\r\n");
        exit(1);
    }

    /* 命令0，关灯，命令1，点灯 */
    if(cmd >= GPIO_CMD)
    {
		printf("GPIO cmd error!\r\n");
		exit(1);
    }
	
	if((num == 0) || (num == 1))
	{
		read_gpio = hian_gpio_device[num];
	}
    
	if((fd = open(read_gpio,O_RDWR|O_NOCTTY|O_NDELAY))<0)
    {
		printf("open %d fail!\r\n",fd);
		exit(1);
	}
	else
	{
		printf("APP open %s success!\n",read_gpio);
		printf("%d gpio value is %d\n",cmd,ioctl(fd,cmd,0));
		//ioctl(fd,cmd,num);			
	}
    
    close(fd);

    return(1);
}
