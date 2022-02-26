/* 任务调度 */

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <mtask.h>

struct schedule {
	long running_task_count;			/* 队列内进程数量 */
	long CPU_exec_task_jiffies;			/* 用于调度时保存进程可用时间片数量 */
	struct task_struct task_queue;		/* 准备就绪的任务队列的队列头 */
};

struct task_struct *now_task;				/* 指向当前进程PCB(仅用于异常和中断处理，不知道为什么，中断时无法通过current宏获得正确的PCB) */
extern struct schedule task_schedule;

void schedule();
void schedule_init();
struct task_struct *get_next_task();
void insert_task_queue(struct task_struct *tsk);

#endif
