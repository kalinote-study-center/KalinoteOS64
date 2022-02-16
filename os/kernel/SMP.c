/* 对称多处理 */
#include <SMP.h>
#include <printk.h>
#include <lib.h>
#include <APIC.h>
#include <asm.h>
#include <types.h>
#include <memory.h>
#include <gate.h>

struct local_APIC_map APU_local_APIC_map;

void APU_LAPIC_pagetable_remap(mem_addr32 ph_addr) {
	/* AP处理器 local APIC 页表映射 */
	color_printk(YELLOW, BLACK, "[SMP]Pagetable remaping:%#010x\t\n",ph_addr);
	
	mem_addr64 *tmp;
	mem_addr8 *LAPIC_addr = (mem_addr8 *)Phy_To_Virt(ph_addr);
	
	APU_local_APIC_map.physical_address = ph_addr;
	APU_local_APIC_map.virtual_index_address  = LAPIC_addr;
	APU_local_APIC_map.virtual_id_address   = (mem_addr32 *)(LAPIC_addr + 0x20);						/* ID寄存器 */
	APU_local_APIC_map.virtual_version_address   = (mem_addr32 *)(LAPIC_addr + 0x30);					/* 版本寄存器 */
	APU_local_APIC_map.virtual_ldr_address   = (mem_addr32 *)(LAPIC_addr + 0xd0);						/* 逻辑目标寄存器LDR */
	APU_local_APIC_map.virtual_dfr_address   = (mem_addr32 *)(LAPIC_addr + 0xe0);						/* 目标格式寄存器DFR */
	APU_local_APIC_map.virtual_svr_address   = (mem_addr32 *)(LAPIC_addr + 0xf0);						/* SVR寄存器 */
	APU_local_APIC_map.virtual_tpr_address	 = (mem_addr32 *)(LAPIC_addr + 0x80);      					/* TPR寄存器 */
	APU_local_APIC_map.virtual_ppr_address   = (mem_addr32 *)(LAPIC_addr + 0xa0);		    			/* PPR寄存器 */
	APU_local_APIC_map.virtual_eoi_address   = (mem_addr32 *)(LAPIC_addr + 0xb0);		    			/* EOI寄存器 */
	APU_local_APIC_map.virtual_lvt_cmci_address   = (mem_addr32 *)(LAPIC_addr + 0x2f0);					/* LVT CMCI寄存器 */
	APU_local_APIC_map.virtual_icr_low_address   = (mem_addr32 *)(LAPIC_addr + 0x300);					/* 中断命令寄存器ICR(0-31bit) */
	APU_local_APIC_map.virtual_icr_high_address   = (mem_addr32 *)(LAPIC_addr + 0x310);					/* 中断命令寄存器ICR(32-63bit) */
	APU_local_APIC_map.virtual_lvt_timer_address	= (mem_addr32 *)(LAPIC_addr + 0x320);				/* LVT 定时器寄存器 */
	APU_local_APIC_map.virtual_lvt_temperature_address	= (mem_addr32 *)(LAPIC_addr + 0x330);			/* LVT 温度寄存器 */
	APU_local_APIC_map.virtual_lvt_perfor_address	= (mem_addr32 *)(LAPIC_addr + 0x340);				/* LVT 性能监控寄存器 */
	APU_local_APIC_map.virtual_lvt_LINT0_address	= (mem_addr32 *)(LAPIC_addr + 0x350);				/* LVT LINT0寄存器 */
	APU_local_APIC_map.virtual_lvt_LINT1_address	= (mem_addr32 *)(LAPIC_addr + 0x360);				/* LVT LINT1寄存器 */
	APU_local_APIC_map.virtual_lvt_err_address	= (mem_addr32 *)(LAPIC_addr + 0x370);					/* LVT 错误寄存器 */
	
	Global_CR3 = Get_gdt();
	
	tmp = Phy_To_Virt(Global_CR3 + (((mem_addr64)LAPIC_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	
	if(*tmp == 0) {
		mem_addr64 *virtual = kmalloc(PAGE_4K_SIZE, 0);
		set_mpl4t(tmp,mk_mpl4t(Virt_To_Phy(virtual),PAGE_KERNEL_GDT));
	}
	
	color_printk(YELLOW, BLACK, "[SMP]1:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);
	
	tmp = Phy_To_Virt((mem_addr64 *)(*tmp & (~ 0xfffUL)) + (((mem_addr64)LAPIC_addr >> PAGE_1G_SHIFT) & 0x1ff));
	if(*tmp == 0) {
		mem_addr64 * virtual = kmalloc(PAGE_4K_SIZE,0);
		set_pdpt(tmp,mk_pdpt(Virt_To_Phy(virtual),PAGE_KERNEL_Dir));
	}
	
	color_printk(YELLOW,BLACK,"[SMP]2:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);
	
	tmp = Phy_To_Virt((mem_addr64 *)(*tmp & (~ 0xfffUL)) + (((mem_addr64)LAPIC_addr >> PAGE_2M_SHIFT) & 0x1ff));
	set_pdt(tmp,mk_pdt(APU_local_APIC_map.physical_address,PAGE_KERNEL_Page | PAGE_PWT | PAGE_PCD));

	color_printk(BLUE,BLACK,"[SMP]3:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);

	color_printk(BLUE,BLACK,"[SMP]APU_local_APIC_map.physical_address:%#010x\t\t\n",APU_local_APIC_map.physical_address);
	color_printk(BLUE,BLACK,"[SMP]APU_local_APIC_map.virtual_address:%#018lx\t\t\n",(mem_addr64)APU_local_APIC_map.virtual_index_address);

	flush_tlb();
}

void SMP_init() {
	int i;
	unsigned int a,b,c,d;

	//get local APIC ID
	for(i = 0;;i++) {
		get_cpuid(0xb,i,&a,&b,&c,&d);
		if((c >> 8 & 0xff) == 0)
			break;
		color_printk(WHITE,BLACK,"[SMP]local APIC ID Package_../Core_2/SMT_1,type(%x) Width:%#010x,num of logical processor(%x)\n",c >> 8 & 0xff,a & 0x1f,b & 0xff);
	}
	
	color_printk(WHITE,BLACK,"[SMP]APIC(x2APIC) ID level:%#010x\tAPIC(x2APIC) ID the current logical processor:%#010x\n",c & 0xff,d);

	color_printk(WHITE,BLACK,"[SMP]SMP copy byte:%#010x\n",(unsigned long)&_APU_boot_end - (unsigned long)&_APU_boot_start);
	memcpy(_APU_boot_start,(unsigned char *)0xffff800000020000,(unsigned long)&_APU_boot_end - (unsigned long)&_APU_boot_start);
}

void Start_SMP() {
	int x,y;

	color_printk(RED,YELLOW,"[SMP]APU Starting...\n");
	
	color_printk(RED,YELLOW,"[SMP]set xAPIC\t\n");
	__asm__ __volatile__(	"movq	$0x1b,	%%rcx	\n\t"
				"rdmsr	\n\t"
				"bts	$11,	%%rax	\n\t"
				"bts	$12,	%%rax	\n\t"		// 修改基地址为0xfee01000
				"wrmsr	\n\t"
				"movq 	$0x1b,	%%rcx	\n\t"
				"rdmsr	\n\t"
				:"=a"(x),"=d"(y)
				:
				:"memory");

	color_printk(RED,YELLOW,"[SMP]eax:%#010x,edx:%#010x\t\n",x,y);

	/* 检查是否成功开启 */
	if(x&0x800) {
		color_printk(RED,YELLOW,"[SMP]AP xAPIC enabled\t\n");
		APU_LAPIC_pagetable_remap(0xfee01000);
	} else {
		color_printk(RED,YELLOW,"[SMP]failed to enable AP APIC&xAPIC\t\n");
		goto loop_hlt;
	}
	
	color_printk(RED, YELLOW, "[SMP]SVR:%#010x\t\n", *APU_local_APIC_map.virtual_svr_address);
	/* 使能SVR第8位，激活APIC(默认在上面设置寄存器时已经激活) */
	*APU_local_APIC_map.virtual_svr_address |= (1 << 8);
	if(*APU_local_APIC_map.virtual_version_address & (1 << 24))	/* 使能SVR第12位 */
		*APU_local_APIC_map.virtual_svr_address |= (1 << 12);
	else
		color_printk(RED, WHITE, "[SMP]Disable broadcast EOI is not support, APIC version reg:%#010x\t\n", *APU_local_APIC_map.virtual_version_address);
	if(*APU_local_APIC_map.virtual_svr_address&0x100)
		color_printk(RED,YELLOW,"[SMP]SVR[8] enabled\t");
	if(*APU_local_APIC_map.virtual_svr_address&0x1000)
		color_printk(RED,YELLOW,"[SMP]SVR[12] enabled\t");
	
	/* 输出ID寄存器 */
	color_printk(RED,YELLOW,"[SMP]APIC&xAPIC ID:%#010x\t\n", *APU_local_APIC_map.virtual_id_address);
	
	load_TR(12);
	
	// x = 1/0;
	
loop_hlt:
	for(;;)
		io_hlt();

}
