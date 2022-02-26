/* 定时器 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <lib.h>
#include <types.h>

extern unsigned long volatile jiffies;

struct timer_list {
	/* 定时器队列(32字节,2M的话可以最多放65536个，实际应该用不了这么多) */
	unsigned long expire_jiffies;
	void (* func)(void * data);
	void *data;
	struct timer_list *next_timer;			/* 指向下一个timer */
};

extern struct timer_list timer_list_last;
extern struct timer_list *timer_list_head;

/* 定时器操作 */
void init_timer(struct timer_list * timer,void (* func)(void * data),void *data,unsigned long expire_jiffies);
void add_timer(struct timer_list * timer);
void del_timer(struct timer_list * timer);

/* 注意区分timer_init和init_timer */
void timer_init();
void do_timer();

#endif
