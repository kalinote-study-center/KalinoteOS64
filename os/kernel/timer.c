/* 定时器处理 */
#include <timer.h>
#include <softirq.h>
#include <printk.h>
#include <lib.h>
#include <memory.h>
#include <mtask.h>
#include <schedule.h>
#include <asm.h>

/* 全局变量 */
unsigned long volatile jiffies = 0;								/* 系统运行tik */
struct timer_list timer_list_last;								/* 这个timer永不过期，永远在队列最后面 */
struct timer_list *timer_list_head = &timer_list_last;
/* 全局变量 */

void test_timer(void *data) {
	/* 为了测试，开机延迟5秒后触发进程调度 */
	color_printk(BLUE,WHITE,"[timer]start_schedule_timer\t\n");
}

void init_timer(struct timer_list * timer,void (* func)(void * data),void *data,unsigned long expire_jiffies) {
	/* 在这里进行初始化赋值，包括链表、超时后执行函数、参数、超时时间 */
	timer->next_timer = NULL;								/* 先把下一个指向NULL */
	timer->func = func;										/* 定义超时时执行的函数 */
	timer->data = data;										/* 函数参数 */
	timer->expire_jiffies = jiffies + expire_jiffies;		/* 定义超时时间 */
}

void add_timer(struct timer_list * timer) {
	/* 
	* 这里对整个链表进行插入操作，设计思路如下：
	* 遍历timer链表的expire_jiffies，为了保持该链表超时时间是从低到高排序
	* 从timer_list_head开始，比较expire_jiffies值，如果比其大，则往后寻找，如果比其小，则插入到之前
	*/
	struct timer_list *tmp = timer_list_head;
	struct timer_list *pre = NULL;
	while(timer->expire_jiffies > tmp->expire_jiffies) {
		pre = tmp;
		tmp = tmp->next_timer;
	}
	if(pre)
		pre->next_timer = timer;
	else
		timer_list_head = timer;
	timer->next_timer = tmp;
}

void del_timer(struct timer_list * timer) {
	struct timer_list *tmp = timer_list_head;
	if(timer == &timer_list_last)		/* timer_list_last不能被删除 */
		return;
	if(timer == timer_list_head) {
		/* 如果第一个就是要被删除的，直接让head指向下一个 */
		timer_list_head = timer_list_head->next_timer;
		return;
	}
	while(tmp->next_timer != timer)		/* 要被删除的是head的情况已经在上面的if中解决了，所以直接从next开始比较 */
		tmp = tmp->next_timer;
	tmp->next_timer = timer->next_timer;
}

void timer_init() {
	jiffies = 0;
	init_timer(&timer_list_last,NULL,NULL,-1UL);		/* 永不过期，永远在最后一个 */
	register_softirq(0,&do_timer,NULL);
	
	struct timer_list *tmp = NULL;
	tmp = (struct timer_list *)kmalloc(sizeof(struct timer_list),0);
	init_timer(tmp,&test_timer,NULL,500);
	add_timer(tmp);
}

void do_timer(void * data) {
	/* 从head开始判断是否有超时，如果head都没有超时，则后面的都不用再判断了(超时时间升序) */
	while(timer_list_head->expire_jiffies <= jiffies) {
		timer_list_head->func(timer_list_head->data);	/* 执行函数 */
		del_timer(timer_list_head);						/* 删除定时器 */
	}
	color_printk(RED,WHITE,"(PIT:%ld)\n",jiffies);
}
