 #define _GNU_SOURCE         
 #include <unistd.h>
 #include <sys/syscall.h>

int hian_add(int a, int b)
{
	return syscall(376, a, b);
}
