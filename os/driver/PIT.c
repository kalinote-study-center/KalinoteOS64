/* 可编程间隔化定时器 */
#include <PIT.h>
#include <time.h>
#include <asm.h>
#include <lib.h>
#include <timer.h>
#include <APIC.h>
#include <interrupt.h>
#include <printk.h>
#include <softirq.h>
#include <mtask.h>
#include <schedule.h>

hw_int_controller PIT_int_controller =  {
	.enable = IOAPIC_enable,
	.disable = IOAPIC_disable,
	.install = IOAPIC_install,
	.uninstall = IOAPIC_uninstall,
	.ack = IOAPIC_edge_ack,
};

void PIT_handler(unsigned long nr, unsigned long parameter, struct pt_regs * regs) {
	jiffies++;

	if(timer_list_head->expire_jiffies <= jiffies)
		set_softirq_status(TIMER_SIRQ);
	
	switch(current->priority) {
		case 0:
		case 1:
			task_schedule.CPU_exec_task_jiffies--;
			current->vrun_time += 1;
			break;
		case 2:
		default:
			task_schedule.CPU_exec_task_jiffies -= 2;
			current->vrun_time += 2;
			break;
	}

	if(task_schedule.CPU_exec_task_jiffies <= 0)
		current->flags |= NEED_SCHEDULE;
}

extern struct time time;

void PIT_init() {
	struct IO_APIC_RET_entry entry;
	
	/*****************************************************************
	*                         PIC控制字定义                          *
	* D7		D6		D5		D4		D3		D2		D1		D0   *
	* -------------------------------------------------------------  *
	*     计数器    |    读写格式    |        工作方式       | 数制  *
	* -------------------------------------------------------------  *
	* SC1		SC0		RL1		RL0		M2		M1		M0		BCD  *
	* -------------------------------------------------------------  *
	*   00 计数器0   00 计数器锁存命令      000    方式0    0 二进制 *
	*   01 计数器1   01 只读写低字节(*)     001    方式1    1 十进制 *
	*   10 计数器2   10 只读写高字节(*)     010    方式2             *
	*   11 非法      11 先读写低字节，      011    方式3             *
	*                   后读写高字节        100    方式4             *
	*                                       101    方式5             *
	*****************************************************************/                                                               
	
	io_out8(PIT_CTRL, 0x34); // 固定值(控制字-计数器0-先读写低字节，后读写高字节-方式2-二进制)
	io_out8(PIT_CNT0, 0x9c); // 中断周期低8位
	io_out8(PIT_CNT0, 0x2e); // 中断周期高8位
	/* 上面发送的终端周期为0x2e9b，换算成十进制是11931，设定这个值的中断频率为100Hz，即每10ms发生一次中断，每秒产生100次中断 */
	
	/* 初始化相关中断处理程序 */
	entry.vector = 0x22;
	entry.deliver_mode = APIC_ICR_IOAPIC_Fixed ;
	entry.dest_mode = ICR_IOAPIC_DELV_PHYSICAL;
	entry.deliver_status = APIC_ICR_IOAPIC_Idle;
	entry.polarity = APIC_IOAPIC_POLARITY_HIGH;
	entry.irr = APIC_IOAPIC_IRR_RESET;
	entry.trigger = APIC_ICR_IOAPIC_Edge;
	entry.mask = APIC_ICR_IOAPIC_Masked;
	entry.reserved = 0;

	entry.destination.physical.reserved1 = 0;
	entry.destination.physical.phy_dest = 0;
	entry.destination.physical.reserved2 = 0;

	register_irq(0x22, &entry , &PIT_handler, NULL, &PIT_int_controller, "PIC");
	
	get_cmos_time(&time);
	color_printk(RED,BLACK,"[PIT]year%#010x,month:%#010x,day:%#010x,hour:%#010x,mintue:%#010x,second:%#010x\n",time.year,time.month,time.day,time.hour,time.minute,time.second);
}
