#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<unistd.h>

static struct wdt_feed_st{
	int time;
};

int main(void)
{
	int fd;
   	char *wdt;

	struct wdt_feed_st wfs;	
	wdt = "/dev/hian_wdt0";

	wfs.time = 15625 * 3;
	
	if((fd = open(wdt,O_RDWR))<0)
    {
		printf("open %d fail!\r\n",fd);
		exit(1);
	}
	
	ioctl(fd, 0, 1);

	while(1)
	{
		write(fd, &wfs, sizeof(wfs));
		sleep(2);
	}
  
    close(fd);

    return 0;
}
