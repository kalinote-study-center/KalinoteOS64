/* 主函数 */

void Start_Kernel(void) {
	/* 测试绘图 */
	int *addr = (int *)0xffff800000a00000;		/* 映射的图形缓冲地址 */
	int i;


	for(i = 0 ;i<1440*20;i++)
	{
		*((char *)addr+0)=(char)0x00;	/* B */
		*((char *)addr+1)=(char)0x00;	/* G */
		*((char *)addr+2)=(char)0xff;	/* R */
		*((char *)addr+3)=(char)0x00;	
		addr +=1;	
	}
	for(i = 0 ;i<1440*20;i++)
	{
		*((char *)addr+0)=(char)0x00;
		*((char *)addr+1)=(char)0xff;
		*((char *)addr+2)=(char)0x00;
		*((char *)addr+3)=(char)0x00;	
		addr +=1;	
	}
	for(i = 0 ;i<1440*20;i++)
	{
		*((char *)addr+0)=(char)0xff;
		*((char *)addr+1)=(char)0x00;
		*((char *)addr+2)=(char)0x00;
		*((char *)addr+3)=(char)0x00;	
		addr +=1;	
	}
	for(i = 0 ;i<1440*20;i++)
	{
		*((char *)addr+0)=(char)0xff;
		*((char *)addr+1)=(char)0xff;
		*((char *)addr+2)=(char)0xff;
		*((char *)addr+3)=(char)0x00;	
		addr +=1;	
	}
	
	/* 暂时是无限循环 */
	while(1);
}
