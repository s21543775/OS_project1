#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/timer.h>

asmlinkage long sys_my_gettime(void)
{
	static const long ns2s = 1000000000;
	struct timespec t;
	getnstimeofday(&t);
	return t.tv_sec*ns2s+t.tv_nsec;
}
