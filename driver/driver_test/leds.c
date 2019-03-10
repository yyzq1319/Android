#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<unistd.h>

#define LED_NUM 2
#define LED_CMD 3

int leds(int num, int cmd)
{
	int fd;
   	char *leds = "/dev/hian_leds";

    /*两个灯，分别为0和1 */
    if(num >= LED_NUM)
    {
		printf("led num error!\r\n");
        exit(1);
    }

    /* 命令0，关灯，命令1，点灯 */
    if(cmd >= LED_CMD)
    {
		printf("led cmd error!\r\n");
		exit(1);
    }

    if((fd = open(leds,O_RDWR|O_NOCTTY|O_NDELAY))<0)
    {
		printf("open %d fail!\r\n",fd);
		exit(1);
	}
	else
    {
		if(2 == cmd)/*1秒闪一次 */
		{
			while(1)
			{
				ioctl(fd,0,num);
				sleep(1);
				ioctl(fd,1,num);
				sleep(1);
			}
		}
		else
		{
			ioctl(fd,cmd,num);			
		}
    }
   
    close(fd);

    return(1);
}
