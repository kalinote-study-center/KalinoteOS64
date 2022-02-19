/* 任务调度 */

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <mtask.h>

struct schedule {
	long running_task_count;
	long CPU_exec_task_jiffies;
	struct task_struct task_queue;
};

struct schedule task_schedule;

void schedule();
void schedule_init();

#endif
