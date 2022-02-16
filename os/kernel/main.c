#include <types.h>
#include <printk.h>
#include <gate.h>
#include <trap.h>
#include <memory.h>
#include <mtask.h>
#include <cpu.h>
#include <graphic.h>
#include <kernel.h>
#include <ptrace.h>
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

struct Global_Memory_Descriptor memory_management_struct = {{0},0};

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
	
	char buf[512];
	int i;
	
	struct INT_CMD_REG icr_entry;
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

	load_TR(10);
	
	set_tss64(_stack_start, _stack_start, _stack_start, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);
	
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
		// APIC_init();
		
		/* 下面是APU启动相关代码，暂时还没有实现相关功能 */
		local_APIC_init();
		SMP_init();
		
		*local_APIC_map.virtual_icr_high_address = 0;
		*local_APIC_map.virtual_icr_low_address = 0xc4500;	//INIT IPI
		*local_APIC_map.virtual_icr_high_address = 0;
		*local_APIC_map.virtual_icr_low_address = 0xc4620;	//Start-up IPI
		*local_APIC_map.virtual_icr_high_address = 0;
		*local_APIC_map.virtual_icr_low_address = 0xc4620;	//Start-up IPI
		
	#else
		init_8259A();
		color_printk(RED,BLACK,"[error]The driver of the 8259A chip is not yet perfect!  \n");
		for(;;)
			io_hlt();
	#endif
	
	// color_printk(RED,BLACK,"keyboard init \n");
	// keyboard_init();
	
	// color_printk(RED,BLACK,"mouse init \n");
	// mouse_init();
	
	// color_printk(RED,BLACK,"disk init \n");
	// disk_init();

	//	color_printk(RED,BLACK,"task_init \n");
	//	task_init();
	// init_screen(printk_info.buf, printk_info.screen_x, printk_info.screen_y);		/* 初始化屏幕图形界面 */

	while(1) {
		// if(p_kb->count)
		// 	analysis_keycode();
		// if(p_mouse->count)
		// 	analysis_mousecode();
		io_hlt();
	}
	
}
