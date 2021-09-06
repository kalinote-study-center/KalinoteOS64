/* CPU */
#include "cpu.h"
#include "printk.h"

void init_cpu(void) {
	int i,j;
	unsigned int CpuFacName[4] = {0,0,0,0};
	char	FactoryName[17] = {0};

	// 获取CPU厂商名称
	get_cpuid(0,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
	*(unsigned int*)&FactoryName[0] = CpuFacName[1];
	*(unsigned int*)&FactoryName[4] = CpuFacName[3];
	*(unsigned int*)&FactoryName[8] = CpuFacName[2];	

	FactoryName[12] = '\0';
	color_printk(COL_YELLOW,COL_BLACK,"%s\t%#010x\t%#010x\t%#010x\n",FactoryName,CpuFacName[1],CpuFacName[3],CpuFacName[2]);
	
	//brand_string
	for(i = 0x80000002;i < 0x80000005;i++) {
		get_cpuid(i,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
		*(unsigned int*)&FactoryName[0] = CpuFacName[0];
		*(unsigned int*)&FactoryName[4] = CpuFacName[1];
		*(unsigned int*)&FactoryName[8] = CpuFacName[2];
		*(unsigned int*)&FactoryName[12] = CpuFacName[3];
		FactoryName[16] = '\0';
		color_printk(COL_YELLOW,COL_BLACK,"%s",FactoryName);
	}
	color_printk(COL_YELLOW,COL_BLACK,"\n");

	// 获取CPU型号相关信息
	get_cpuid(1,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
	color_printk(COL_YELLOW,COL_BLACK,"Family Code:%#010x,Extended Family:%#010x,Model Number:%#010x,Extended Model:%#010x,Processor Type:%#010x,Stepping ID:%#010x\n",(CpuFacName[0] >> 8 & 0xf),(CpuFacName[0] >> 20 & 0xff),(CpuFacName[0] >> 4 & 0xf),(CpuFacName[0] >> 16 & 0xf),(CpuFacName[0] >> 12 & 0x3),(CpuFacName[0] & 0xf));

	// 获取线性/物理地址大小
	get_cpuid(0x80000008,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
	color_printk(COL_YELLOW,COL_BLACK,"Physical Address size:%08d,Linear Address size:%08d\n",(CpuFacName[0] & 0xff),(CpuFacName[0] >> 8 & 0xff));

	// 最大CPUID操作码
	get_cpuid(0,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
	color_printk(COL_WHITE,COL_BLACK,"MAX Basic Operation Code :%#010x\t",CpuFacName[0]);

	get_cpuid(0x80000000,0,&CpuFacName[0],&CpuFacName[1],&CpuFacName[2],&CpuFacName[3]);
	color_printk(COL_WHITE,COL_BLACK,"MAX Extended Operation Code :%#010x\n",CpuFacName[0]);


}
