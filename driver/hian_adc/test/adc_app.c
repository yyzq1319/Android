#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<unistd.h>

static struct adc_value_st{
	int value;
};

int main(void)
{
	int fd;
   	char *adc;
	int ret;

	struct adc_value_st avs;	
	adc = "/dev/hian_adc0";

	
	//if((fd = open(adc, O_RDONLY | O_NONBLOCK))<0)
	if((fd = open(adc, O_RDONLY))<0)
    {
		printf("open %d fail!\r\n",fd);
		exit(1);
	}
	
	while(1)
	{
		ioctl(fd, 0, 0);
		ret = read(fd, &avs, sizeof(avs));
		if(ret <0)
		{
			perror("read");
			exit(1);
		}
		printf("adc_value = %d!\r\n", avs.value);	
		sleep(2);	
	}
  
    close(fd);

    return 0;
}
