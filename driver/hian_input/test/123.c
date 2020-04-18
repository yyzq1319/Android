#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <sys/epoll.h>
#include <assert.h>
 
#define CNT 3
 
void get_event(int fd)
{
	int ret;
	struct input_event event;//对输入设备封装一个结构体 
	
	printf("------------ get_event ----------\n");
 
 	ret = read(fd, &event, sizeof(event));//

 	switch (event.type) {///事件类型的分类
		case EV_SYN:
			printf("------------ syn ----------\n");
	        break;
 		case EV_KEY:
 			printf("key code%d is %s!\n", event.code,//例如某个事件（按键类型）类型里众多按键里相应按键的编码
			event.value?"down":"up");//该事件里某个按键发生的值：按下/按上
	        break;
		default:
			break;
	}
}
  
void add_to_epfd(int epfd, int fd)
{
	int ret;
  
	struct epoll_event event = {//定义一个感兴趣的事件结构体对象
				.events = EPOLLIN, //a.感兴趣的事件/对应的文件描述符可读
				.data    = {           //b.保存触发事件的文件描述符相关的数据
				.fd = fd,
				},
			};    
  
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);//2.epoll的事件注册函数
	//1返回值/注册新的fd到epfd/需要监听的fd/告诉内核需要监听什么事
  
	assert(ret == 0);
}

int main(int argc, char **argv)
{
	int epfd, fd;
	int ret;
	int i;
	struct epoll_event events[CNT];//参数events是分配好的epoll_event结构体数组，epoll将会把发生的事件赋值到events数组中
  
	#define LEN 64
	char buf[LEN];
    
	printf("hehe, this is me...\n");
  
	epfd = epoll_create(CNT);//1.创建一个epoll的句柄,指向内核一段空间，对应struct file结构体
    assert(epfd > 0);
  
    for (i = 0; i < CNT; i++) 
	{
    	snprintf(buf, LEN, "/dev/input/event%d", i);
    	fd = open(buf, O_RDONLY);
    	add_to_epfd(epfd, fd);
    }        
  
    while (1) 
	{
    	ret = epoll_wait(epfd, events, CNT, -1);//3.收集在epoll监控的事件中已经发送的事件/返回值是满足条件的前几个文件描述符
         //1返回值/感兴趣事件/感兴趣事件大小为4/不确定或者永久等待,一直阻塞等待，睡着。。。
		if (ret < 0) 
		{
        	perror("epoll_wait");
        	exit(1);
        }
		else if (ret == 0) 
		{
			//-1:立马返回/有超时时间（2000ms）：显示超时
            printf("hehe, timeout...\n");
            continue;
		}
 
		for (i = 0; i < ret; i++) 
		{
        	if (events[i].events&EPOLLIN) 
			{
				//获取满足条件的文件描述符对应的元素的结构体
            	get_event(events[i].data.fd);
            }
        }
	}

	return 0;
}
