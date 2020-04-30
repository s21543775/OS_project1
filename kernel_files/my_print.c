#include<linux/linkage.h>
#include<linux/kernel.h>

asmlinkage void sys_my_print(int pid, long start_time, long end_time)
{
	static const long ns2s = 1000000000;
	printk(KERN_INFO "[Project1] %d %ld.%09ld %ld.%09ld", pid, start_time / ns2s, start_time % ns2s,end_time / ns2s,end_time % ns2s);
	printk("\n");
}
