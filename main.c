#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "process.h"
//#include "scheduler.h"

static int t_last;
static int ntime;
static int running;
static int finish_cnt;

int cmp(const void *a, const void *b)
{
	return ((struct process *)a)->t_ready - ((struct process *)b)->t_ready;
}

int scheduling(struct process *proc, int numofproc, char* stringpolicy);
int next_process(struct process *proc, int numofproc, char* stringpolicy);

int main()
{

	char stringpolicy[64];
	int numofproc;
	struct process *proc;

	scanf("%s%d", stringpolicy,&numofproc);
	
	proc = (struct process *)malloc(numofproc * sizeof(struct process));

	for (int i = 0; i < numofproc; i++)
		scanf("%s%d%d", proc[i].name,&proc[i].t_ready, &proc[i].t_exec);
	scheduling(proc, numofproc, stringpolicy);

	exit(0);
}

int scheduling(struct process *proc, int numofproc, char* stringpolicy)
{
	qsort(proc, numofproc, sizeof(struct process), cmp);

	/* Initial pid = -1 imply not ready */
	for (int i = 0; i < numofproc; i++)
		proc[i].pid = -1;

	/* Set single core prevent from preemption */
	proc_assign_cpu(getpid(), PARENT_CPU);
	
	/* Set high priority to scheduler */
	proc_wakeup(getpid());
	
	/* Initial scheduler */
	ntime = 0;
	running = -1;
	finish_cnt = 0;
	
	while(1) {
		//fprintf(stderr, "Current time: %d\n", ntime);
			int printpid = 0;
			if(running!= -1)
				printpid = proc[running].pid;
		/* Check if running process finish */
		if (running != -1 && proc[running].t_exec == 0) {
		
#ifdef DEBUG
			//fprintf(stderr, "%s finish at time %d.\n", proc[running].name, ntime);
#endif
			//kill(running, SIGKILL);
			//printf("hello\n");
			waitpid(proc[running].pid, NULL, WUNTRACED);
			printpid = proc[running].pid;	
			printf("%s %d\n", proc[running].name, proc[running].pid);
			running = -1;
			finish_cnt++;

			/* All process finish */
			if (finish_cnt == numofproc)
				break;
		}

		/* Check if process ready and execute */
		for (int i = 0; i < numofproc; i++) {
			if (proc[i].t_ready == ntime) {
				proc[i].pid = proc_exec(proc[i],printpid);
				proc_block(proc[i].pid);
#ifdef DEBUG
				//fprintf(stderr, "%s ready at time %d.\n", proc[i].name, ntime);
#endif
			}

		}
		/* Select next running  process */
		int next = next_process(proc, numofproc, stringpolicy);
		//printf("%d\n",next);
		if (next != -1) {
			/* Context switch */
			if (running != next) {
				proc_wakeup(proc[next].pid);
				if(running != -1)
					proc_block(proc[running].pid);
				running = next;
				t_last = ntime;
			}
		}

		/* Run an unit of time */
		UNIT_T();
		if (running != -1)
			proc[running].t_exec--;
		//printf("running = %d, t_exec = %d\n", running, proc[running].t_exec);
		ntime++;
	}

	return 0;
}
/* Return index of next process  */
int next_process(struct process *proc, int numofproc, char* stringpolicy)
{
	/* Non-preemptive */
	if (running != -1 && (strcmp(stringpolicy,"SJF")==0 || strcmp(stringpolicy,"FIFO")==0))
		return running;

	int ret = -1;

	if (strcmp(stringpolicy,"PSJF")==0 || strcmp(stringpolicy,"SJF")==0)
	{
		for (int i = 0; i < numofproc; i++) {
			if (proc[i].pid == -1 || proc[i].t_exec == 0)
				continue;
			if (ret == -1 || proc[i].t_exec < proc[ret].t_exec)
				ret = i;
		}
	}

	else if (strcmp(stringpolicy,"FIFO")==0)
	{
		for(int i = 0; i < numofproc; i++) {
			if(proc[i].pid == -1 || proc[i].t_exec == 0)
				continue;
			if(ret == -1 || proc[i].t_ready < proc[ret].t_ready)
				ret = i;
		}
	}

	else if (strcmp(stringpolicy,"RR")==0)
	{
		if (running == -1)
		{
			ret = -1;
			for(int i=0;i<numofproc;i++)
			{
				if(proc[i].pid!=-1&&proc[i].t_exec>0)
				{
					if(ret == -1)
					{
						ret = i;
						continue;
					}
				
					if(proc[i].t_ready < proc[ret].t_ready)
						ret = i;
					else if(proc[i].t_ready == proc[ret].t_ready)
						if(i<ret)
							ret = i;
				}
			}
		}
		else if ((ntime - t_last) % 500 == 0)
		{
			proc[running].t_ready = ntime;
			ret = -1;
			for(int i=0;i<numofproc;i++)
			{
				if(proc[i].pid == -1||proc[i].t_exec == 0)
					continue;
				if(ret == -1)
				{
					ret = i;
					continue;
				}
				if(proc[i].t_ready < proc[ret].t_ready)
					ret = i;
				else if(proc[i].t_ready == proc[ret].t_ready)
				{
					if(i<ret)
						ret = i;
				}
			}
		}
		else
			ret = running;
	}

	return ret;
}
