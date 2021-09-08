#include "lib.h"
#include "printk.h"
#include "gate.h"
#include "trap.h"
#include "memory.h"
#include "task.h"
#include "cpu.h"

#define APIC	1

#if  APIC
#include "APIC.h"
#else
#include "8259A.h"
#endif

/* 图形缓冲区映射地址为0xffff800003000000 */
#include "keyboard.h"
#include "mouse.h"
#include "disk.h"
#include "SMP.h"

struct Global_Memory_Descriptor memory_management_struct = {{0},0};

void KaliKernel(void) {
	/* KalinoteOS2.0 内核程序入口 */
	struct INT_CMD_REG icr_entry;

	Pos.XResolution = 1440;
	Pos.YResolution = 900;

	Pos.XPosition = 0;
	Pos.YPosition = 0;

	Pos.XCharSize = 8;
	Pos.YCharSize = 16;

	Pos.FB_addr = (int *)0xffff800003000000;
	Pos.FB_length = (Pos.XResolution * Pos.YResolution * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK;
	
	load_TR(10);

	set_tss64(_stack_start, _stack_start, _stack_start, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);

	sys_vector_init();

	// init_cpu();

	memory_management_struct.start_code = (unsigned long)& _text;
	memory_management_struct.end_code   = (unsigned long)& _etext;
	memory_management_struct.end_data   = (unsigned long)& _edata;
	memory_management_struct.end_brk    = (unsigned long)& _end;


	// i = 1/0;	/* 除0异常 */
	// i = *(int *)0xffff80000aa00000;		/* 页异常 */

	color_printk(COL_RED,COL_BLACK,"memory init \n");
	init_memory();

	color_printk(COL_RED,COL_BLACK,"slab init \n");
	slab_init();

	color_printk(COL_RED,COL_BLACK,"frame buffer init \n");
	frame_buffer_init();
	color_printk(COL_WHITE,COL_BLACK,"frame_buffer_init() is OK \n");

	color_printk(COL_RED,COL_BLACK,"pagetable init \n");	
	pagetable_init();
	
	Local_APIC_init();

	color_printk(COL_RED,COL_BLACK,"ICR init \n");	

	SMP_init();
	
	icr_entry.vector = 0x00;
	icr_entry.deliver_mode =  APIC_ICR_IOAPIC_INIT;
	icr_entry.dest_mode = ICR_IOAPIC_DELV_PHYSICAL;
	icr_entry.deliver_status = APIC_ICR_IOAPIC_Idle;
	icr_entry.res_1 = 0;
	icr_entry.level = ICR_LEVEL_DE_ASSERT;
	icr_entry.trigger = APIC_ICR_IOAPIC_Edge;
	icr_entry.res_2 = 0;
	icr_entry.dest_shorthand = ICR_ALL_EXCLUDE_Self;
	icr_entry.res_3 = 0;
	icr_entry.destination.x2apic_destination = 0x00;
	
	wrmsr(0x830,*(unsigned long *)&icr_entry);	//INIT IPI

	icr_entry.vector = 0x20;
	icr_entry.deliver_mode = ICR_Start_up;
	
	wrmsr(0x830,*(unsigned long *)&icr_entry);	//Start-up IPI
	wrmsr(0x830,*(unsigned long *)&icr_entry);	//Start-up IPI

	while(1);
}
