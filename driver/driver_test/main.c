#include <stdio.h>  
#include<string.h>
#include<stdlib.h>
#include"leds.h"

int main(int argc, char *argv[]) 
{  
	int ret = 0;

    if((0 == strcmp("leds",argv[1])))
	{
		
		leds(atoi(argv[2]),atoi(argv[3]));
	}
    else
    {
		printf("input argv[1] %s error!!!\r\n",argv[1]);
    }

	return 0;  
}
