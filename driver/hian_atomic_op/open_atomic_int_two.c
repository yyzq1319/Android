#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc,char **argv){
	char *atomic_op = "/dev/hian_atomic_op";
	int fd;
	
	if((fd = open(atomic_op,O_RDWR|O_NDELAY))<0){
		printf("%s open %s failed!\n",argv[0],atomic_op);
	}
	else{
		printf("%s open %s sucess!\n",argv[0],atomic_op);
	}
	
	while(1);
}
