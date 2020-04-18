#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<linux/input.h>

struct input_event event;

void event_value_check(int value)
{
    printf("event value check!\r\n");
	
	if(value)
	{
		printf("event.code = %d!!!\r\n", event.code);
	}
	else
	{
		printf("event.code = %d!\r\n", event.code);
	}
}

void event_code_check(int value)
{
	if(value == ABS_X)
	{
		printf("abs_x:%d\r\n", event.value);
	}
	else if(value == ABS_Y)
	{
		printf("abs_y:%d\r\n", event.value);
	}
	else if(value == REL_X)
	{
		printf("rel_x:%d\r\n", event.value);
	}
	else if(value == REL_Y)
	{
		printf("rel_y:%d\r\n", event.value);
	}
	
}


int main(int argc, char *argv[])
{
	int fd;
	struct input_event event;
	
	if(argc != 2)
	{
		fprintf(stderr, "input error\r\n");
		exit(0);
	}

	fd = open(argv[1], O_RDONLY);

	while(1)
	{
		read(fd, &event, sizeof(event));
        
		switch(event.type)
		{
			case EV_SYN:
				printf("syn event!\r\n");
				break;
			case EV_KEY:
				printf("event.code =  %d\r\n", event.code);
				//event_value_check(event.value);
				break;
			case EV_ABS: //绝对事件
				event_code_check(event.code);
				break;
			case EV_REL: //相对事件，比如鼠标
				event_code_check(event.code);
				break;
			default:
				printf("misc event!\r\n");
				break;
		}
	}	

    return 0;
}
