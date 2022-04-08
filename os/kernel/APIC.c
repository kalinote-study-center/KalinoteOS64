/* 高级中断管理 */
#include <types.h>
#include <APIC.h>
#include <CPU.h>
#include <printk.h>
#include <ptrace.h>
#include <asm.h>
#include <memory.h>
#include <gate.h>
#include <interrupt.h>
#include <pci.h>
#include <kernel.h>

void IOAPIC_enable(unsigned long irq) {
	unsigned long value = 0;
	value = ioapic_rte_read((irq - 32) * 2 + 0x10);
	value = value & (~0x10000UL); 
	ioapic_rte_write((irq - 32) * 2 + 0x10,value);
}

void IOAPIC_disable(unsigned long irq) {
	unsigned long value = 0;
	value = ioapic_rte_read((irq - 32) * 2 + 0x10);
	value = value | 0x10000UL; 
	ioapic_rte_write((irq - 32) * 2 + 0x10,value);
}

unsigned long IOAPIC_install(unsigned long irq,void * arg) {
	struct IO_APIC_RET_entry *entry = (struct IO_APIC_RET_entry *)arg;
	ioapic_rte_write((irq - 32) * 2 + 0x10,*(unsigned long *)entry);

	return 1;
}

void IOAPIC_uninstall(unsigned long irq) {
	ioapic_rte_write((irq - 32) * 2 + 0x10,0x10000UL);
}

void IOAPIC_level_ack(unsigned long irq) {
	*local_APIC_map.virtual_eoi_address = 0x0;
	*ioapic_map.virtual_EOI_address = irq;
}

void IOAPIC_edge_ack(unsigned long irq) {
	*local_APIC_map.virtual_eoi_address = 0x0;
}

extern struct local_APIC_map APU_local_APIC_map[4];

void Local_APIC_edge_level_ack(unsigned long irq) {
	/* 该函数存疑 */
	// int i;
	*local_APIC_map.virtual_eoi_address = 0x0;
	// /* 好像不能这么处理 */
	// for(i = 0; i < 4; i++)	/* 向所有APU发送中断处理完毕的信号(不知道能不能这么处理) */
	// 	*APU_local_APIC_map[i].virtual_eoi_address = 0x0;
	// *APU_local_APIC_map[0].virtual_eoi_address = 0x0;	/* 这一句仅用于定向测试 */
	// *APU_local_APIC_map[1].virtual_eoi_address = 0x0;	/* 这一句仅用于定向测试 */
}

void LAPIC_pagetable_remap() {
	/* local APIC 页表映射 */
	mem_addr64 *tmp;
	mem_addr8 *LAPIC_addr = (mem_addr8 *)Phy_To_Virt(0xfee00000);
	
	local_APIC_map.physical_address = 0xfee00000;
	local_APIC_map.virtual_index_address  = LAPIC_addr;
	local_APIC_map.virtual_id_address   = (mem_addr32 *)(LAPIC_addr + 0x20);						/* ID寄存器 */
	local_APIC_map.virtual_version_address   = (mem_addr32 *)(LAPIC_addr + 0x30);					/* 版本寄存器 */
	local_APIC_map.virtual_ldr_address   = (mem_addr32 *)(LAPIC_addr + 0xd0);						/* 逻辑目标寄存器LDR */
	local_APIC_map.virtual_dfr_address   = (mem_addr32 *)(LAPIC_addr + 0xe0);						/* 目标格式寄存器DFR */
	local_APIC_map.virtual_svr_address   = (mem_addr32 *)(LAPIC_addr + 0xf0);						/* SVR寄存器 */
	local_APIC_map.virtual_tpr_address	 = (mem_addr32 *)(LAPIC_addr + 0x80);      					/* TPR寄存器 */
	local_APIC_map.virtual_ppr_address   = (mem_addr32 *)(LAPIC_addr + 0xa0);		    			/* PPR寄存器 */
	local_APIC_map.virtual_eoi_address   = (mem_addr32 *)(LAPIC_addr + 0xb0);		    			/* EOI寄存器 */
	local_APIC_map.virtual_lvt_cmci_address   = (mem_addr32 *)(LAPIC_addr + 0x2f0);					/* LVT CMCI寄存器 */
	local_APIC_map.virtual_icr_low_address   = (mem_addr32 *)(LAPIC_addr + 0x300);					/* 中断命令寄存器ICR(0-31bit) */
	local_APIC_map.virtual_icr_high_address   = (mem_addr32 *)(LAPIC_addr + 0x310);					/* 中断命令寄存器ICR(32-63bit) */
	local_APIC_map.virtual_lvt_timer_address	= (mem_addr32 *)(LAPIC_addr + 0x320);				/* LVT 定时器寄存器 */
	local_APIC_map.virtual_lvt_temperature_address	= (mem_addr32 *)(LAPIC_addr + 0x330);			/* LVT 温度寄存器 */
	local_APIC_map.virtual_lvt_perfor_address	= (mem_addr32 *)(LAPIC_addr + 0x340);				/* LVT 性能监控寄存器 */
	local_APIC_map.virtual_lvt_LINT0_address	= (mem_addr32 *)(LAPIC_addr + 0x350);				/* LVT LINT0寄存器 */
	local_APIC_map.virtual_lvt_LINT1_address	= (mem_addr32 *)(LAPIC_addr + 0x360);				/* LVT LINT1寄存器 */
	local_APIC_map.virtual_lvt_err_address	= (mem_addr32 *)(LAPIC_addr + 0x370);					/* LVT 错误寄存器 */
	
	Global_CR3 = Get_gdt();
	
	tmp = Phy_To_Virt(Global_CR3 + (((mem_addr64)LAPIC_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	
	if(*tmp == 0) {
		mem_addr64 *virtual = kmalloc(PAGE_4K_SIZE, 0);
		memset(virtual,0,PAGE_4K_SIZE);
		set_mpl4t(tmp,mk_mpl4t(Virt_To_Phy(virtual),PAGE_KERNEL_GDT));
	}
	
	color_printk(YELLOW, BLACK, "[APIC]1:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);
	
	tmp = Phy_To_Virt((mem_addr64 *)(*tmp & (~ 0xfffUL)) + (((mem_addr64)LAPIC_addr >> PAGE_1G_SHIFT) & 0x1ff));
	if(*tmp == 0) {
		mem_addr64 * virtual = kmalloc(PAGE_4K_SIZE,0);
		memset(virtual,0,PAGE_4K_SIZE);
		set_pdpt(tmp,mk_pdpt(Virt_To_Phy(virtual),PAGE_KERNEL_Dir));
	}
	
	color_printk(YELLOW,BLACK,"[APIC]2:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);
	
	tmp = Phy_To_Virt((mem_addr64 *)(*tmp & (~ 0xfffUL)) + (((mem_addr64)LAPIC_addr >> PAGE_2M_SHIFT) & 0x1ff));
	set_pdt(tmp,mk_pdt(local_APIC_map.physical_address,PAGE_KERNEL_Page | PAGE_PWT | PAGE_PCD));

	color_printk(BLUE,BLACK,"[APIC]3:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);

	color_printk(BLUE,BLACK,"[APIC]local_APIC_map.physical_address:%#010x\t\t\n",local_APIC_map.physical_address);
	color_printk(BLUE,BLACK,"[APIC]local_APIC_map.virtual_address:%#018lx\t\t\n",(mem_addr64)local_APIC_map.virtual_index_address);

	flush_tlb();
}

void IOAPIC_pagetable_remap() {
	/* IO APIC 页表映射 */
	mem_addr64 * tmp;
	mem_addr8 * IOAPIC_addr = (mem_addr8 *)Phy_To_Virt(0xfec00000);

	ioapic_map.physical_address = 0xfec00000;
	ioapic_map.virtual_index_address  = IOAPIC_addr;
	ioapic_map.virtual_data_address   = (mem_addr32 *)(IOAPIC_addr + 0x10);
	ioapic_map.virtual_EOI_address    = (mem_addr32 *)(IOAPIC_addr + 0x40);
	
	Global_CR3 = Get_gdt();

	tmp = Phy_To_Virt(Global_CR3 + (((mem_addr64)IOAPIC_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	if (*tmp == 0) {
		mem_addr64 * virtual = kmalloc(PAGE_4K_SIZE,0);
		memset(virtual,0,PAGE_4K_SIZE);
		set_mpl4t(tmp,mk_mpl4t(Virt_To_Phy(virtual),PAGE_KERNEL_GDT));
	}

	color_printk(YELLOW,BLACK,"[APIC]1:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);

	tmp = Phy_To_Virt((mem_addr64 *)(*tmp & (~ 0xfffUL)) + (((mem_addr64)IOAPIC_addr >> PAGE_1G_SHIFT) & 0x1ff));
	if(*tmp == 0) {
		mem_addr64 * virtual = kmalloc(PAGE_4K_SIZE,0);
		memset(virtual,0,PAGE_4K_SIZE);
		set_pdpt(tmp,mk_pdpt(Virt_To_Phy(virtual),PAGE_KERNEL_Dir));
	}

	color_printk(YELLOW,BLACK,"[APIC]2:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);
	
	tmp = Phy_To_Virt((mem_addr64 *)(*tmp & (~ 0xfffUL)) + (((mem_addr64)IOAPIC_addr >> PAGE_2M_SHIFT) & 0x1ff));
	set_pdt(tmp,mk_pdt(ioapic_map.physical_address,PAGE_KERNEL_Page | PAGE_PWT | PAGE_PCD));

	color_printk(BLUE,BLACK,"[APIC]3:%#018lx\t%#018lx\t\n",(mem_addr64)tmp,(mem_addr64)*tmp);

	color_printk(BLUE,BLACK,"[APIC]ioapic_map.physical_address:%#010x\t\t\n",ioapic_map.physical_address);
	color_printk(BLUE,BLACK,"[APIC]ioapic_map.virtual_address:%#018lx\t\t\n",(mem_addr64)ioapic_map.virtual_index_address);

	flush_tlb();
}

unsigned long ioapic_rte_read(unsigned char index) {
	unsigned long ret;

	*ioapic_map.virtual_index_address = index + 1;
	io_mfence();
	ret = *ioapic_map.virtual_data_address;
	ret <<= 32;
	io_mfence();

	*ioapic_map.virtual_index_address = index;		
	io_mfence();
	ret |= *ioapic_map.virtual_data_address;
	io_mfence();

	return ret;
}

void ioapic_rte_write(unsigned char index,unsigned long value) {
	*ioapic_map.virtual_index_address = index;
	io_mfence();
	*ioapic_map.virtual_data_address = value & 0xffffffff;
	value >>= 32;
	io_mfence();
	
	*ioapic_map.virtual_index_address = index + 1;
	io_mfence();
	*ioapic_map.virtual_data_address = value & 0xffffffff;
	io_mfence();
}

void local_APIC_init() {
	/* local APIC 初始化 */
	unsigned int x,y;
	unsigned int a,b,c,d;
	
	get_cpuid(1,0,&a,&b,&c,&d);
	color_printk(WHITE,BLACK,"[APIC]CPUID\t01,eax:%#010x,ebx:%#010x,ecx:%#010x,edx:%#010x\t\n",a,b,c,d);
	
	if((1<<9) & d) {
		color_printk(WHITE,BLACK,"[APIC]HW support APIC&xAPIC\t");
		apic_info.xapic = 1;
	} else {
		color_printk(WHITE,BLACK,"[APIC]HW NO support APIC&xAPIC\t");
		apic_info.xapic = 0;
	}
	if((1<<21) & c) {
		color_printk(WHITE,BLACK,"HW support x2APIC\t\n");
		apic_info.x2apic = 1;
	} else {
		color_printk(WHITE,BLACK,"HW NO support x2APIC\t\n");
		apic_info.x2apic = 0;
	}
	
	if(!apic_info.xapic) {
		/* 不支持APIC&xAPIC */
		color_printk(RED, WHITE, "[APIC]init APIC failed: device not support\t\n");
		return;
	}
	
	LAPIC_pagetable_remap();	/* 页表映射 */
	
	if (apic_info.x2apic) {
		/* 支持x2APIC */
		color_printk(WHITE,BLACK,"set x2APIC\t\n");
		__asm__ __volatile__(	"movq	$0x1b,	%%rcx	\n\t"
					"rdmsr	\n\t"
					"bts	$10,	%%rax	\n\t"
					"bts	$11,	%%rax	\n\t"
					"wrmsr	\n\t"
					"movq 	$0x1b,	%%rcx	\n\t"
					"rdmsr	\n\t"
					:"=a"(x),"=d"(y)
					:
					:"memory");
	} else {
		/* 在仅支持xAPIC的平台设置第10位(保留位)会触发通用保护异常 */
		/* https://wiki.osdev.org/APIC#Local_APIC_and_IO-APIC */
		color_printk(WHITE,BLACK,"set xAPIC\t\n");
		__asm__ __volatile__(	"movq	$0x1b,	%%rcx	\n\t"
					"rdmsr	\n\t"
					"bts	$11,	%%rax	\n\t"
					"wrmsr	\n\t"
					"movq 	$0x1b,	%%rcx	\n\t"
					"rdmsr	\n\t"
					:"=a"(x),"=d"(y)
					:
					:"memory");
	}
	
	color_printk(WHITE,BLACK,"[APIC]eax:%#010x,edx:%#010x\t\n",x,y);
	
	/* 检查是否成功开启 */
	if(x&0x800)
		color_printk(WHITE,BLACK,"[APIC]xAPIC enabled\t");
	else {
		color_printk(RED,WHITE,"[APIC]failed to enable APIC&xAPIC\t\n");
		return;
	}
	if(x&0x400)
		color_printk(WHITE,BLACK,"x2APIC enabled\t");
	color_printk(BLACK,BLACK,"\n");
	
	/* 根据英特尔开发文档卷3A Part1中第10章的相关说明，APIC(xAPIC)和x2APIC对于寄存器有不同的操作模式 */
	/* APIC(xAPIC)采用内存地址映射的方式来操控寄存器，而x2APIC将寄存器映射到了MSR空间中 */
	/* 其中，英特尔开发文档卷3A Part1的Table 10-1中说明了各寄存器映射到对应的物理地址 */
	/* Table 10-6说明了各寄存器对应的MSR空间地址 */
	/* 所以这里对于不同版本的APIC需要进行不同的处理 */
	
	if(apic_info.x2apic) {
		/* 如果支持x2APIC */
		/* 由于我所使用的测试平台对x2APIC兼容性不高，所以暂时仅对APIC&xAPIC进行支持 */
		/* 从文档上来看，x2APIC应该也是支持xAPIC这一套的 */
		/* x2APIC对寄存器的操作依赖于RDMSR和WRMSR指令 */
		//enable SVR[8]
		__asm__ __volatile__(	"movq 	$0x80f,	%%rcx	\n\t"
					"rdmsr	\n\t"
					"bts	$8,	%%rax	\n\t"
					// "bts	$12,	%%rax\n\t"
					"wrmsr	\n\t"
					"movq 	$0x80f,	%%rcx	\n\t"
					"rdmsr	\n\t"
					:"=a"(x),"=d"(y)
					:
					:"memory");
	
		color_printk(WHITE,BLACK,"[APIC]SVR set: eax:%#010x,edx:%#010x\t\n",x,y);
		
		if(x&0x100)
			color_printk(WHITE,BLACK,"[APIC]SVR[8] enabled\t");
		if(x&0x1000)
			color_printk(WHITE,BLACK,"[APIC]SVR[12] enabled\t");
		color_printk(BLACK,BLACK,"\n");
	
		//get local APIC ID
		__asm__ __volatile__(	"movq $0x802,	%%rcx	\n\t"
					"rdmsr	\n\t"
					:"=a"(x),"=d"(y)
					:
					:"memory");
		
		color_printk(WHITE,BLACK,"[APIC]eax:%#010x,edx:%#010x\tx2APIC ID:%#010x\n",x,y,x);
		
		//get local APIC version
		__asm__ __volatile__(	"movq $0x803,	%%rcx	\n\t"
					"rdmsr	\n\t"
					:"=a"(x),"=d"(y)
					:
					:"memory");
					
		color_printk(WHITE,BLACK,"local APIC Version:%#010x,Max LVT Entry:%#010x,SVR(Suppress EOI Broadcast):%#04x\t",x & 0xff,(x >> 16 & 0xff) + 1,x >> 24 & 0x1);
		
		if((x & 0xff) < 0x10)
			color_printk(WHITE,BLACK,"82489DX discrete APIC\n");
		else if( ((x & 0xff) >= 0x10) && ((x & 0xff) <= 0x15) )
			color_printk(WHITE,BLACK,"Integrated APIC\n");
		
		//mask all LVT	
		__asm__ __volatile__(	//"movq 	$0x82f,	%%rcx	\n\t"	//CMCI
					//"wrmsr	\n\t"
					"movq 	$0x832,	%%rcx	\n\t"	//Timer
					"wrmsr	\n\t"
					"movq 	$0x833,	%%rcx	\n\t"	//Thermal Monitor
					"wrmsr	\n\t"
					"movq 	$0x834,	%%rcx	\n\t"	//Performance Counter
					"wrmsr	\n\t"
					"movq 	$0x835,	%%rcx	\n\t"	//LINT0
					"wrmsr	\n\t"
					"movq 	$0x836,	%%rcx	\n\t"	//LINT1
					"wrmsr	\n\t"
					"movq 	$0x837,	%%rcx	\n\t"	//Error
					"wrmsr	\n\t"
					:
					:"a"(0x10000),"d"(0x00)
					:"memory");
	
		color_printk(GREEN,BLACK,"[APIC]Mask ALL LVT\n");
	
		//TPR
		__asm__ __volatile__(	"movq 	$0x808,	%%rcx	\n\t"
					"rdmsr	\n\t"
					:"=a"(x),"=d"(y)
					:
					:"memory");
	
		color_printk(GREEN,BLACK,"[APIC]Set LVT TPR:%#010x\t",x);
	
		//PPR
		__asm__ __volatile__(	"movq 	$0x80a,	%%rcx	\n\t"
					"rdmsr	\n\t"
					:"=a"(x),"=d"(y)
					:
					:"memory");
	
		color_printk(GREEN,BLACK,"Set LVT PPR:%#010x\n",x);
		return;
	}
	
	if(apic_info.xapic) {
		/* 如果支持xAPIC */
		/* APIC(xAPIC)对寄存器的操作，通过操作映射到内存中的值来操作寄存器，默认地址为0xfee00000(需映射到线性内存地址，可手动修改) */
		/* APIC(xAPIC)不支持直接使用RDMSR和WRMSR指令对寄存器进行修改，会触发#GP异常 */
		color_printk(RED, WHITE, "[APIC]SVR:%#010x\t\n", *local_APIC_map.virtual_svr_address);
		
		/* 使能SVR第8位，激活APIC(默认在上面设置寄存器时已经激活) */
		*local_APIC_map.virtual_svr_address |= (1 << 8);
		/* 使能SVR第12位，禁止广播EOI(开发环境bochs不支持) */
		/* 通过版本寄存器的第24位检测是否支持禁用广播EOI消息功能 */
		if(*local_APIC_map.virtual_version_address & (1 << 24))	/* 使能SVR第12位 */
			*local_APIC_map.virtual_svr_address |= (1 << 12);
		else
			color_printk(RED, WHITE, "[APIC]Disable broadcast EOI is not support, APIC version reg:%#010x\t\n", *local_APIC_map.virtual_version_address);
		
		if(*local_APIC_map.virtual_svr_address&0x100)
			color_printk(WHITE,BLACK,"[APIC]SVR[8] enabled\t");
		if(*local_APIC_map.virtual_svr_address&0x1000)
			color_printk(WHITE,BLACK,"[APIC]SVR[12] enabled\t");
		color_printk(BLACK,BLACK,"\n");
		
		/* 输出ID寄存器 */
		color_printk(WHITE,BLACK,"[APIC]APIC&xAPIC ID:%#010x\t\n", *local_APIC_map.virtual_id_address);
		
		/* 输出版本寄存器及相关信息 */
		color_printk(WHITE,BLACK,"[APIC]APIC&xAPIC Version:%#010x\t\n", *local_APIC_map.virtual_version_address);
		if((*local_APIC_map.virtual_version_address & 0xff) < 0x10)
			color_printk(WHITE,BLACK,"[APIC]82489DX discrete APIC\t\n");
		else if( ((*local_APIC_map.virtual_version_address & 0xff) >= 0x10) && ((*local_APIC_map.virtual_version_address & 0xff) <= 0x15) )
			color_printk(WHITE,BLACK,"[APIC]Integrated APIC\t\n");
		
		/* 屏蔽所有LVT(暂时)，第16位置1 */
		if(platform.platform != PLATFORM_BOCHS)
			*local_APIC_map.virtual_lvt_cmci_address   = 0x10000;		/* LVT CMCI寄存器(bochs不支持) */
		*local_APIC_map.virtual_lvt_timer_address	= 0x10000;			/* LVT 定时器寄存器 */
		*local_APIC_map.virtual_lvt_temperature_address	= 0x10000;		/* LVT 温度寄存器 */
		*local_APIC_map.virtual_lvt_perfor_address	= 0x10000;			/* LVT 性能监控寄存器 */
		*local_APIC_map.virtual_lvt_LINT0_address	= 0x10000;			/* LVT LINT0寄存器 */
		*local_APIC_map.virtual_lvt_LINT1_address	= 0x10000;			/* LVT LINT1寄存器 */
		*local_APIC_map.virtual_lvt_err_address	= 0x10000;				/* LVT 错误寄存器 */
		color_printk(GREEN,BLACK,"[APIC]Mask ALL LVT\t\n");
		
		// TPR寄存器、PPR寄存器
		color_printk(GREEN,BLACK,"[APIC]Set LVT TPR:%#010x\t",*local_APIC_map.virtual_tpr_address);
		color_printk(GREEN,BLACK,"[APIC]Set LVT PPR:%#010x\t\n",*local_APIC_map.virtual_ppr_address);	
	}
}

void IOAPIC_init() {
	/* IO APIC初始化 */
	int i ;
	//	I/O APIC
	//	I/O APIC	ID	
	*ioapic_map.virtual_index_address = 0x00;
	io_mfence();
	*ioapic_map.virtual_data_address = 0x0f000000;
	io_mfence();
	color_printk(GREEN,BLACK,"[APIC]Get IOAPIC ID REG:%#010x,ID:%#010x\t\n",*ioapic_map.virtual_data_address, *ioapic_map.virtual_data_address >> 24 & 0xf);
	io_mfence();

	//	I/O APIC	Version
	*ioapic_map.virtual_index_address = 0x01;
	io_mfence();
	color_printk(GREEN,BLACK,"[APIC]Get IOAPIC Version REG:%#010x,MAX redirection enties:%#08d\t\n",*ioapic_map.virtual_data_address ,((*ioapic_map.virtual_data_address >> 16) & 0xff) + 1);

	//RTE	
	for(i = 0x10;i < 0x40;i += 2)
		ioapic_rte_write(i,0x10020 + ((i - 0x10) >> 1));

	ioapic_rte_write(0x12, 0x21);
	
	color_printk(GREEN,BLACK,"[APIC]I/O APIC Redirection Table Entries Set Finished.\t\n");	
}

void APIC_init() {
	int i;
	unsigned int x;
	unsigned int * p;
		
	IOAPIC_pagetable_remap();		/* 映射IO APIC内存 */
		
	for(i = 32; i < 56; i++) {
		set_intr_gate(i , 2 , interrupt[i - 32]);
	}
	
	// 屏蔽 8259A中断芯片
	color_printk(GREEN,BLACK,"[APIC]MASK 8259A\n");
	io_out8(0x21,0xff);
	io_out8(0xa1,0xff);
	
	// 使能 IMCR
	io_out8(0x22,0x70);
	io_out8(0x23,0x01);	
	
	local_APIC_init();		/* 初始化local APIC */
	IOAPIC_init();			/* 初始化IOAPIC */
	
	// 获取RCBA地址(RCBA位于bus 0, device 31, function 0, offset F0h)
	io_out32(CONFIG_ADDRESS, make_pci_address(0, 31, 0, 0xf0));
	x = io_in32(CONFIG_DATA);
	color_printk(RED,BLACK,"[APIC]Get RCBA Address:%#010x\t\n",x);
	/* 为什么这里获取到的x是0xffffffff */
	
	//获取 RCBA 地址
	io_out32(0xcf8,0x8000f8f0);
	x = io_in32(0xcfc);
	color_printk(RED,BLACK,"[APIC]Get RCBA Address:%#010x\t\n",x);	
	x = x & 0xffffc000;
	color_printk(RED,BLACK,"[APIC]Get RCBA Address:%#010x\t\n",x);	

	//获取 OIC 地址
	if(x > 0xfec00000 && x < 0xfee00000) {
		p = (unsigned int *)Phy_To_Virt(x + 0x31feUL);
	}
	
	// for(;;);
	/* 这里由于获取不到RCBA的地址(读出值为0xffffffff)，所以无法设置OIC */
	/* 但是经过测试，这里不进行设置也没有影响到中断功能 */
	/* 所以暂时不对此处进行处理 */
	//enable IOAPIC
	// x = (*p & 0xffffff00) | 0x100;
	// io_mfence();
	// *p = x;
	// io_mfence();

	memset(interrupt_desc,0,sizeof(irq_desc_T)*NR_IRQS);
	
	/* bochs平台在这里执行sti会报#DF(双重错误), 但是不执行无法接受中断 */
	/* Hyper-V没有此问题 */
	/* 怀疑是BOCHS平台的问题，因为从开始到现在已经遇到过很多次类似情况了 */
	/* TODO：需要进一步检查并解决 */
	// io_sti();
}

void do_IRQ(struct pt_regs * regs,unsigned long nr)	{
	switch(nr & 0x80) {
		case 0x00:	/* 普通中断信号 */
		{
			irq_desc_T * irq = &interrupt_desc[nr - 32];
			if(irq->handler != NULL)
				irq->handler(nr,irq->parameter,regs);
			if(irq->controller != NULL && irq->controller->ack != NULL)
				irq->controller->ack(nr);
		}
			break;
		case 0x80:	/* IPI通信信号 */
			color_printk(RED,BLACK,"[APIC]SMP IPI :%d\n",nr);
			Local_APIC_edge_level_ack(nr);
			break;
		default:
			color_printk(RED,BLACK,"[APIC]do_IRQ receive:%d\n",nr);
			break;
	}
}
