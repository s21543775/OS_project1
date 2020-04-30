#define _GNU_SOURCE
#include "process.h"
#include <sched.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#define GET_TIME 314
#define PRINTK 315

int proc_assign_cpu(int pid, int core)
{
	if (core > sizeof(cpu_set_t)) {
		fprintf(stderr, "Core index error.");
		return -1;
	}

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);
		
	if (sched_setaffinity(pid, sizeof(mask), &mask) < 0) {
		//perror("sched_setaffinity");
		exit(1);
	}

	return 0;
}

int proc_exec(struct process proc,int printpid)
{
	int pid = fork();
	if(pid > 0)
		proc_assign_cpu(pid, CHILD_CPU);
	if (pid < 0) {
		//perror("fork");
		return -1;
	}

	if (pid == 0) 
	{
		usleep(1000);
		long start_time, end_time;
		char to_dmesg[200];
		//printf("123\n");
		start_time = syscall(333);
		for (int i = 0; i < proc.t_exec; i++) {
			UNIT_T();
#ifdef DEBUG
			//if (i % 100 == 0)
				//fprintf(stderr, "%s: %d/%d\n", proc.name, i, proc.t_exec);
#endif
		}
		//printf("Hi\n");
		end_time = syscall(333);
		//sprintf(to_dmesg, "[project1] %d %lu.%09lu %lu.%09lu\n", getpid(), start_sec, start_nsec, end_sec, end_nsec);
		//pid_t oldpid = getpid();
		syscall(334,getpid(),start_time,end_time,11111);
		//printf("goodbye\n");
		exit(0);
	}
	
	/* Assign child to another core prevent from interupted by parant */

	return pid;
}

int proc_block(int pid)
{
	struct sched_param param;
	
	/* SCHED_IDLE should set priority to 0 */
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_IDLE, &param);
	
	if (ret < 0) {
		//perror("sched_setscheduler");
		return -1;
	}
	//printf("before TSTP\n");
	kill(pid,SIGTSTP);
	//printf("after TSTP\n");
	return ret;
}

int proc_wakeup(int pid)
{
	struct sched_param param;
	
	/* SCHED_OTHER should set priority to 0 */
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_OTHER, &param);
	
	if (ret < 0) {
		//perror("sched_setscheduler");
		return -1;
	}
	kill(pid,SIGCONT);
	return ret;
}
