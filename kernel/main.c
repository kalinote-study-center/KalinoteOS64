#include "lib.h"
#include "printk.h"
#include "gate.h"
#include "trap.h"
#include "memory.h"

extern char _text;
extern char _etext;
extern char _edata;
extern char _end;

struct Global_Memory_Descriptor memory_management_struct = {{0},0};

void KaliKernel(void) {
	/* KalinoteOS2.0 内核程序入口 */
	int *addr = (int *)0xffff800000a00000;
	int i;

	Pos.XResolution = 1440;
	Pos.YResolution = 900;

	Pos.XPosition = 0;
	Pos.YPosition = 0;

	Pos.XCharSize = 8;
	Pos.YCharSize = 16;

	Pos.FB_addr = (int *)0xffff800000a00000;
	Pos.FB_length = (Pos.XResolution * Pos.YResolution * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK;

	color_printk(COL_RED,COL_GREEN,"KalinoteOS2.0!\n");
	
	load_TR(8);

	set_tss64(0xffff800000007c00,
	0xffff800000007c00,
	0xffff800000007c00,
	0xffff800000007c00,
	0xffff800000007c00,
	0xffff800000007c00,
	0xffff800000007c00,
	0xffff800000007c00,
	0xffff800000007c00,
	0xffff800000007c00);
	
	sys_vector_init();

	memory_management_struct.start_code = (unsigned long)& _text;
	memory_management_struct.end_code   = (unsigned long)& _etext;
	memory_management_struct.end_data   = (unsigned long)& _edata;
	memory_management_struct.end_brk    = (unsigned long)& _end;


	// i = 1/0;	/* 除0异常 */
	// i = *(int *)0xffff80000aa00000;		/* 页异常 */

	color_printk(COL_RED,COL_BLACK,"memory init \n");
	init_memory();

	color_printk(COL_RED,COL_BLACK,"interrupt init \n");
	init_interrupt();

	while(1);
}
