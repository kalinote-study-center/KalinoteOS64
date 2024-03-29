/* 键盘控制器驱动 */
#include <keyboard.h>
#include <lib.h>
#include <asm.h>
#include <interrupt.h>
#include <memory.h>
#include <printk.h>
#include <APIC.h>

struct keyboard_buffer * p_kb = NULL;
static int shift_l,shift_r,ctrl_l,ctrl_r,alt_l,alt_r;	/* 控制键状态 */

void keyboard_exit() {
	/* 卸载键盘驱动程序 */
	unregister_irq(0x21);
	kfree((unsigned long *)p_kb);
}

void keyboard_handler(unsigned long nr, unsigned long parameter, struct pt_regs *regs) {
	/* 键盘中断处理程序 */
	unsigned char x;
	x = io_in8(0x60);
	
	// color_printk(WHITE, BLACK, "(K:%02x)\n",x);
	
	if(p_kb->p_head == p_kb->buf + KB_BUF_SIZE)
		p_kb->p_head = p_kb->buf;

	*p_kb->p_head = x;
	p_kb->count++;
	p_kb->p_head ++;
}

unsigned char get_scancode() {
	unsigned char ret  = 0;

	if(p_kb->count == 0)
		while(!p_kb->count)
			io_nop();
	
	if(p_kb->p_tail == p_kb->buf + KB_BUF_SIZE)	
		p_kb->p_tail = p_kb->buf;

	ret = *p_kb->p_tail;
	p_kb->count--;
	p_kb->p_tail++;

	return ret;
}

void analysis_keycode() {
	unsigned char x = 0;
	int i;	
	int key = 0;	
	int make = 0;

	x = get_scancode();
	
	if(x == 0xE1) {
		//pause break;
		key = PAUSEBREAK;
		for(i = 1;i<6;i++)
			if(get_scancode() != pausebreak_scode[i]) {
				key = 0;
				break;
			}
	}	
	else if(x == 0xE0) {
		//print screen
		x = get_scancode();

		switch(x) {
			case 0x2A: // press printscreen
		
				if(get_scancode() == 0xE0)
					if(get_scancode() == 0x37) {
						key = PRINTSCREEN;
						make = 1;
					}
				break;

			case 0xB7: // UNpress printscreen
		
				if(get_scancode() == 0xE0)
					if(get_scancode() == 0xAA) {
						key = PRINTSCREEN;
						make = 0;
					}
				break;

			case 0x1d: // press right ctrl
		
				ctrl_r = 1;
				key = OTHERKEY;
				break;

			case 0x9d: // UNpress right ctrl
		
				ctrl_r = 0;
				key = OTHERKEY;
				break;
			
			case 0x38: // press right alt
		
				alt_r = 1;
				key = OTHERKEY;
				break;

			case 0xb8: // UNpress right alt
		
				alt_r = 0;
				key = OTHERKEY;
				break;		

			default:
				key = OTHERKEY;
				break;
		}
		
	}
	
	if(key == 0) {
		unsigned int * keyrow = NULL;
		int column = 0;

		make = (x & FLAG_BREAK ? 0:1);

		keyrow = &keycode_map_normal[(x & 0x7F) * MAP_COLS];

		if(shift_l || shift_r)
			column = 1;

		key = keyrow[column];
		
		switch(x & 0x7F) {
			case 0x2a:	//SHIFT_L:
				shift_l = make;
				key = 0;
				break;

			case 0x36:	//SHIFT_R:
				shift_r = make;
				key = 0;
				break;

			case 0x1d:	//CTRL_L:
				ctrl_l = make;
				key = 0;
				break;

			case 0x38:	//ALT_L:
				alt_l = make;
				key = 0;
				break;

			default:
				if(!make)
					key = 0;
				break;
		}			

		// if(key)
		// 	color_printk(RED,BLACK,"(K:%c)\t",key);
	}
}

hw_int_controller keyboard_int_controller = {
	.enable = IOAPIC_enable,
	.disable = IOAPIC_disable,
	.install = IOAPIC_install,
	.uninstall = IOAPIC_uninstall,
	.ack = IOAPIC_edge_ack,
};

void keyboard_init() {
	/* 键盘初始化 */
	struct IO_APIC_RET_entry entry;
	unsigned long i,j;
	
	/* 键盘数据缓冲区设置 */
	p_kb = (struct keyboard_buffer *)kmalloc(sizeof(struct keyboard_buffer),0);
	
	p_kb->p_head = p_kb->buf;
	p_kb->p_tail = p_kb->buf;
	p_kb->count  = 0;
	memset(p_kb->buf,0,KB_BUF_SIZE);
	
	entry.vector = 0x21;
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
	
	/* 键盘控制器初始化 */
	wait_KB_write();
	io_out8(PORT_KB_CMD,KBCMD_WRITE_CMD);
	wait_KB_write();
	io_out8(PORT_KB_DATA,KB_INIT_MODE);
	
	for(i = 0;i<1000;i++)
		for(j = 0;j<1000;j++)
			io_nop();
	
	/* 初始化控制键状态 */
	shift_l = 0;
	shift_r = 0;
	ctrl_l  = 0;
	ctrl_r  = 0;
	alt_l   = 0;
	alt_r   = 0;
	
	register_irq(0x21, &entry , &keyboard_handler, (unsigned long)p_kb, &keyboard_int_controller, "ps/2 keyboard");
}

