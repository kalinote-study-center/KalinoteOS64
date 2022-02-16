/* 对称多处理 */
#include <SMP.h>
#include <printk.h>
#include <lib.h>
#include <APIC.h>
#include <asm.h>
#include <types.h>
#include <memory.h>

struct local_APIC_map APU_local_APIC_map;

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

	color_printk(RED,YELLOW,"[SMP]APU starting......\n");

	for(;;)
		io_hlt();

}
