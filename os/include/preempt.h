/* 抢占机制 */
#ifndef __PREEMPT_H__
#define __PREEMPT_H__

#include <mtask.h>

#define preempt_enable()		\
do					\
{					\
	now_task[0]->preempt_count--;	\
}while(0)

#define preempt_disable()		\
do					\
{					\
	now_task[0]->preempt_count++;	\
}while(0)

#endif
