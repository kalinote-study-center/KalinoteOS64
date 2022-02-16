/* 8259A中断芯片控制 */
/*
* 对此文件的简单说明：
* 由于使用8259A芯片只能使用单处理器，后续没有办法开发SMP(多处理器)
* 所以后续还会尝试开发APIC驱动的开发
*/
#include <printk.h>
#include <8259A.h>
#include <ptrace.h>
#include <interrupt.h>
#include <asm.h>
#include <gate.h>
#include <lib.h>

void init_8259A() {
	int i;

	for(i = 32;i < 56;i++) {
	/* 注册中断 */
		set_intr_gate(i , 2 , interrupt[i - 32]);
	}

	color_printk(RED,BLACK,"8259A init \n");

	//8259A-master	ICW1-4 初始化
	io_out8(0x20,0x11);
	io_out8(0x21,0x20);
	io_out8(0x21,0x04);
	io_out8(0x21,0x01);

	//8259A-slave	ICW1-4
	io_out8(0xa0,0x11);
	io_out8(0xa1,0x28);
	io_out8(0xa1,0x02);
	io_out8(0xa1,0x01);

	//8259A-M/S	OCW1
	io_out8(0x21,0xf9);		/* 开启鼠标和键盘 */
	io_out8(0xa1,0xef);

	io_sti();
}

void do_IRQ(struct pt_regs * regs,unsigned long nr)	{
	
}
