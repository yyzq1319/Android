#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
 
 
#define  N  128
 
int main(int argc, const char *argv[])
{
	int fd;
 
	char buf[N] = {};
	char wbuf[N] = "This is a write test.";
	pid_t pid;
 
	//fd = open("/dev/hello", O_RDWR|O_NONBLOCK);
	fd = open("/dev/hian_debug", O_RDWR);
	if(fd < 0)
	{
		perror("Failed to open.");
		return -1;
	}
	else
	{
		printf("open success.\n");
	}
 
	if((pid = fork()) < 0)
	{
		perror("Failed to fork.");
		return -1;
	}
	else if(pid == 0)
	{
		if(read(fd, buf, N) < 0)
		{
			perror("Failed to read");
			return -1;
		}
		printf("buf:%s\n", buf);
	}
	else
	{
		sleep(5);
		write(fd, wbuf, strlen(wbuf)+1);
		printf("Wrote done.\n");
	
	}
	close(fd);
 
 
	return 0;
}
