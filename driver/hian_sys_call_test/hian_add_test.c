#include <stdio.h>
#include "hian_add.h"
int main (void)
{
	int sum = 0;
	sum = hian_add(2,3);
	printf("sys_call_test:sum = %d\r\n",sum);
	return 0;
}
