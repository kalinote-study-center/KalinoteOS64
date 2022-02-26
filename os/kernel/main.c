#include <types.h>
#include <printk.h>
#include <gate.h>
#include <trap.h>
#include <memory.h>
#include <mtask.h>
#include <cpu.h>
#include <graphic.h>
#include <kernel.h>
#include <asm.h>

#if APIC
#include <APIC.h>
#else
#include <8259A.h>
#endif

#include <keyboard.h>
#include <mouse.h>
#include <disk.h>
#include <SMP.h>
#include <spinlock.h>
#include <PIT.h>
#include <timer.h>

void KaliKernel(void) {
	/* KalinoteOS64 内核程序入口 */
	/*
	* 内核程序起始地址0xffff800001000000
	* Hyper-V的VRAM地址是0xf8000000
	* bochs的VRAM地址是0xe0000000
	* VMware的VRAM地址是0xe8000000
	* 已经将0xab000000映射到0xffff8000ab000000(其中，ab是具体地址变量)
	* 如果更改VRAM地址，需要对printk中的frame_buffer_init函数进行相应修改
	*/
	unsigned int * tss = NULL;
	/* 配置printk_info相关数据 */
	printk_info.mode = 0;			/* 文字命令模式 */
	#if HYPER_V
		platform.platform = PLATFORM_HYPER_V;
		printk_info.screen_x = 1024;
		printk_info.screen_y = 768;
	#endif
	#if BOCHS
		platform.platform = PLATFORM_BOCHS;
		printk_info.screen_x = 1440;
		printk_info.screen_y = 900;
	#endif
	printk_info.buf = (vram_addr *)0xffff800003000000;
	printk_info.pos_x = 0;
	printk_info.pos_y = 0;
	printk_info.csize_x = 8;
	printk_info.csize_y = 16;
	printk_info.buf_length = (printk_info.screen_x * printk_info.screen_y * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK;
	
	color_printk(RED,WHITE,"[Hello]Welcome to use KalinoteOS64! \n");

	spin_init(&printk_info.printk_lock);

	load_TR(10);
	
	set_tss64(TSS64_Table, _stack_start, _stack_start, _stack_start, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);
	
	sys_vector_init();

	init_cpu();

	memory_management_struct.start_code = (unsigned long)& _text;
	memory_management_struct.end_code   = (unsigned long)& _etext;
	memory_management_struct.end_data   = (unsigned long)& _edata;
	memory_management_struct.end_brk    = (unsigned long)& _end;

	color_printk(RED,BLACK,"[init]memory init \n");
	init_memory();
	
	color_printk(RED,BLACK,"[init]slab init \n");
	slab_init();
	
	color_printk(RED,BLACK,"[init]frame buffer init \n");
	frame_buffer_init();
	color_printk(WHITE,BLACK,"[init]frame_buffer_init() is OK \n");
	color_printk(WHITE,BLACK,"[init]printk_info.buf:%#018x\n",printk_info.buf);

	color_printk(RED,BLACK,"[init]pagetable init \n");	
	pagetable_init();
	
	color_printk(RED,BLACK,"[init]interrupt init \n");
	#if APIC
		APIC_init();		
	#else
		init_8259A();
		color_printk(RED,BLACK,"[error]The driver of the 8259A chip is not yet perfect!  \n");
		for(;;)
			io_hlt();
	#endif
	color_printk(RED,BLACK,"[init]schedule init \n");
	schedule_init();
	
	color_printk(RED,BLACK,"[init]Soft IRQ init \n");
	softirq_init();
	
	color_printk(RED,BLACK,"[init]keyboard init \n");
	keyboard_init();
	
	color_printk(RED,BLACK,"[init]mouse init \n");
	mouse_init();

	color_printk(RED,BLACK,"[init]ICR init \n");	
	SMP_init();
	
	*local_APIC_map.virtual_icr_high_address = 0;
	*local_APIC_map.virtual_icr_low_address = 0xc4500;	//INIT IPI
	
	for(global_i = 1;global_i < 8;) {
		spin_lock(&SMP_lock);
		
		_stack_start = (unsigned long)kmalloc(STACK_SIZE,0) + STACK_SIZE;
		tss = (unsigned int *)kmalloc(128,0);
		set_tss_descriptor(10 + global_i * 2,tss);
		set_tss64(tss,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start);
	
		*local_APIC_map.virtual_icr_high_address = (global_i << 24);		/* 指定投递目标 */
		*local_APIC_map.virtual_icr_low_address = 0x620;	//Start-up IPI
		*local_APIC_map.virtual_icr_high_address = (global_i << 24);
		*local_APIC_map.virtual_icr_low_address = 0x620;	//Start-up IPI
	}
	
	*local_APIC_map.virtual_icr_high_address = (1 << 24);	/* 目标处理器为1号APU */
	*local_APIC_map.virtual_icr_low_address = 0xc8;
	*local_APIC_map.virtual_icr_high_address = (1 << 24);	/* 目标处理器为1号APU */
	*local_APIC_map.virtual_icr_low_address = 0xc9;
	
	while(global_i < 8)		/* 为了防止混乱，等待AP处理器初始化结束再继续执行 */
		io_hlt();

	color_printk(RED,BLACK,"[init]timer init \n");
	timer_init();
	
	color_printk(RED,BLACK,"[init]PIT init \n");
	PIT_init();

	color_printk(RED,BLACK,"[init]task init \n");
	io_sti();
	task_init();

	// init_screen(printk_info.buf, printk_info.screen_x, printk_info.screen_y);		/* 初始化屏幕图形界面 */
	color_printk(BLACK,WHITE,"[init]current:%018lx,current->vrun_time:%ld\t\n",current, current->vrun_time);

	while(1) {
		if(p_kb->count)
			analysis_keycode();
		if(p_mouse->count)
			analysis_mousecode();
	}
	
}
