/* 内核打印相关功能，用于非图形界面的模式 */
#include <stdarg.h>
#include <printk.h>
#include <types.h>
#include <lib.h>
#include <fonts.h>
#include <memory.h>
#include <kernel.h>

char printk_buf[4096] = {0};

void frame_buffer_init() {
	////re init frame buffer;
	mem_addr64 i;
	mem_addr64 * tmp;
	mem_addr64 * tmp1;
	mem_addr32 phy_addr;
	
	if(platform.platform == PLATFORM_HYPER_V)
		phy_addr = 0xf8000000;
	else if (platform.platform == PLATFORM_BOCHS)
		phy_addr = 0xe0000000;
	
	mem_addr32 * FB_addr = (mem_addr32 *)Phy_To_Virt(phy_addr);

	Global_CR3 = Get_gdt();

	tmp = Phy_To_Virt((mem_addr64 *)((mem_addr64)Global_CR3 & (~ 0xfffUL)) + (((mem_addr64)FB_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	if (*tmp == 0) {
		mem_addr64 * virtual = kmalloc(PAGE_4K_SIZE,0);
		set_mpl4t(tmp,mk_mpl4t(Virt_To_Phy(virtual),PAGE_KERNEL_GDT));
	}

	tmp = Phy_To_Virt((mem_addr64 *)(*tmp & (~ 0xfffUL)) + (((mem_addr64)FB_addr >> PAGE_1G_SHIFT) & 0x1ff));
	if(*tmp == 0) {
		mem_addr64 * virtual = kmalloc(PAGE_4K_SIZE,0);
		set_pdpt(tmp,mk_pdpt(Virt_To_Phy(virtual),PAGE_KERNEL_Dir));
	}
	
	for(i = 0;i < printk_info.buf_length;i += PAGE_2M_SIZE) {
		tmp1 = Phy_To_Virt((mem_addr64 *)(*tmp & (~ 0xfffUL)) + (((mem_addr64)((mem_addr64)FB_addr + i) >> PAGE_2M_SHIFT) & 0x1ff));
	
		mem_addr64 phy = phy_addr + i;
		set_pdt(tmp1,mk_pdt(phy,PAGE_KERNEL_Page | PAGE_PWT | PAGE_PCD));
	}

	printk_info.buf = (vram_addr *)Phy_To_Virt(phy_addr);

	flush_tlb();
}

void putchar(vram_addr *vram, int xsize, int x, int y, color_code FColor, color_code BColor, font_type font) {
	/* 在指定位置绘制一个字符 */
	/*
	* 参数说明
	* vram - 图像缓冲区地址指针
	* xsize - 显示区域的x(宽度)尺寸
	* x - x坐标
	* y - y坐标
	* FColor - 字体颜色
	* BColor - 背景颜色
	* font - 需要打印的字符
	*/
	int i = 0, j = 0;
	vram_addr *addr = NULL;
	font_type *fontp = NULL;
	int testval = 0;
	fontp = font_ascii[font];
	
	for(i = 0; i< 16;i++)
	{
		addr = vram + xsize * ( y + i ) + x;
		testval = 0x100;
		for(j = 0;j < 8;j ++)		
		{
			testval = testval >> 1;
			if(*fontp & testval)
				*addr = FColor;
			else
				*addr = BColor;
			addr++;
		}
		fontp++;		
	}
}

int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

static char * number(char * str, long num, int base, int size, int precision,	int type)
{
	char c,sign,tmp[50];
	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&SMALL) digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (type&LEFT) type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ' ;
	sign = 0;
	if (type&SIGN && num < 0) {
		sign='-';
		num = -num;
	} else
		sign=(type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
	if (sign) size--;
	if (type & SPECIAL)
		if (base == 16) size -= 2;
		else if (base == 8) size--;
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num!=0)
		tmp[i++]=digits[do_div(num,base)];
	if (i > precision) precision=i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while(size-- > 0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL)
		if (base == 8)
			*str++ = '0';
		else if (base==16) 
		{
			*str++ = '0';
			*str++ = digits[33];
		}
	if (!(type & LEFT))
		while(size-- > 0)
			*str++ = c;

	while(i < precision--)
		*str++ = '0';
	while(i-- > 0)
		*str++ = tmp[i];
	while(size-- > 0)
		*str++ = ' ';
	return str;
}


int vsprintf(char * buf,const char *fmt, va_list args)
{
	char * str,*s;
	int flags;
	int field_width;
	int precision;
	int len,i;

	int qualifier;		/* 'h', 'l', 'L' or 'Z' for integer fields */

	for(str = buf; *fmt; fmt++)
	{

		if(*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}
		flags = 0;
		repeat:
			fmt++;
			switch(*fmt)
			{
				case '-':flags |= LEFT;	
				goto repeat;
				case '+':flags |= PLUS;	
				goto repeat;
				case ' ':flags |= SPACE;	
				goto repeat;
				case '#':flags |= SPECIAL;	
				goto repeat;
				case '0':flags |= ZEROPAD;	
				goto repeat;
			}

			/* get field width */

			field_width = -1;
			if(is_digit(*fmt))
				field_width = skip_atoi(&fmt);
			else if(*fmt == '*')
			{
				fmt++;
				field_width = va_arg(args, int);
				if(field_width < 0)
				{
					field_width = -field_width;
					flags |= LEFT;
				}
			}
			
			/* get the precision */

			precision = -1;
			if(*fmt == '.')
			{
				fmt++;
				if(is_digit(*fmt))
					precision = skip_atoi(&fmt);
				else if(*fmt == '*')
				{	
					fmt++;
					precision = va_arg(args, int);
				}
				if(precision < 0)
					precision = 0;
			}
			
			qualifier = -1;
			if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
			{	
				qualifier = *fmt;
				fmt++;
			}
							
			switch(*fmt)
			{
				case 'c':

					if(!(flags & LEFT))
						while(--field_width > 0)
							*str++ = ' ';
					*str++ = (unsigned char)va_arg(args, int);
					while(--field_width > 0)
						*str++ = ' ';
					break;

				case 's':
				
					s = va_arg(args,char *);
					if(!s)
						s = '\0';
					len = strlen(s);
					if(precision < 0)
						precision = len;
					else if(len > precision)
						len = precision;
					
					if(!(flags & LEFT))
						while(len < field_width--)
							*str++ = ' ';
					for(i = 0;i < len ;i++)
						*str++ = *s++;
					while(len < field_width--)
						*str++ = ' ';
					break;

				case 'o':
					
					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),8,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),8,field_width,precision,flags);
					break;

				case 'p':

					if(field_width == -1)
					{
						field_width = 2 * sizeof(void *);
						flags |= ZEROPAD;
					}

					str = number(str,(unsigned long)va_arg(args,void *),16,field_width,precision,flags);
					break;

				case 'x':

					flags |= SMALL;

				case 'X':

					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),16,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),16,field_width,precision,flags);
					break;

				case 'd':
				case 'i':

					flags |= SIGN;
				case 'u':

					if(qualifier == 'l')
						str = number(str,va_arg(args,unsigned long),10,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),10,field_width,precision,flags);
					break;

				case 'n':
					
					if(qualifier == 'l')
					{
						long *ip = va_arg(args,long *);
						*ip = (str - buf);
					}
					else
					{
						int *ip = va_arg(args,int *);
						*ip = (str - buf);
					}
					break;

				case '%':
					
					*str++ = '%';
					break;

				default:

					*str++ = '%';	
					if(*fmt)
						*str++ = *fmt;
					else
						fmt--;
					break;
			}

	}
	*str = '\0';
	return str - buf;
}

void putchars_asc(vram_addr *vram, int xsize, int x, int y, color_code FColor, color_code BColor, font_type *s) {
	/* 以ascii码打印字符串 */
	/*
	* 参数说明
	* vram - 图像缓冲区地址指针
	* xsize - 显示区域的x(宽度)尺寸
	* x - x坐标
	* y - y坐标
	* FColor - 字体颜色
	* BColor - 背景颜色
	* font - 需要打印的字符串
	*/
	for(; *s != 0x00; s++){
		putchar(vram, xsize, x, y, FColor, BColor, *s);
		x+=8;
	}
}

int color_printk(color_code FRcolor,color_code BKcolor,const char * fmt,...) {
	/* 该函数用于无图形界面的模式 */
	int i = 0;
	int count = 0;
	int line = 0;
	va_list args;
	va_start(args, fmt);

	i = vsprintf(printk_buf,fmt, args);

	va_end(args);

	if(printk_info.mode)		/* 如果处于图形模式，则禁用内核打印 */
		return 0;
	
	for(count = 0;count < i || line;count++)
	{
		////	add \n \b \t
		if(line > 0)
		{
			count--;
			goto Label_tab;
		}
		if((font_type)*(printk_buf + count) == '\n')
		{
			printk_info.pos_y++;
			printk_info.pos_x = 0;
		}
		else if((font_type)*(printk_buf + count) == '\b')
		{
			printk_info.pos_x--;
			if(printk_info.pos_x < 0)
			{
				printk_info.pos_x = (printk_info.screen_x / printk_info.csize_x - 1) * printk_info.csize_x;
				printk_info.pos_y--;
				if(printk_info.pos_y < 0)
					printk_info.pos_y = (printk_info.screen_y / printk_info.csize_y - 1) * printk_info.csize_y;
			}	
			putchar(printk_info.buf  , printk_info.screen_x , printk_info.pos_x * printk_info.csize_x , printk_info.pos_y * printk_info.csize_y , FRcolor , BKcolor , ' ');	
		}
		else if((font_type)*(printk_buf + count) == '\t')
		{
			line = ((printk_info.pos_x + 8) & ~(8 - 1)) - printk_info.pos_x;

Label_tab:
			line--;
			putchar(printk_info.buf  , printk_info.screen_x , printk_info.pos_x * printk_info.csize_x , printk_info.pos_y * printk_info.csize_y , FRcolor , BKcolor , ' ');	
			printk_info.pos_x++;
		}
		else
		{
			putchar(printk_info.buf  , printk_info.screen_x , printk_info.pos_x * printk_info.csize_x , printk_info.pos_y * printk_info.csize_y , FRcolor , BKcolor , (font_type)*(printk_buf + count));
			printk_info.pos_x++;
		}


		if(printk_info.pos_x >= (printk_info.screen_x / printk_info.csize_x))
		{
			printk_info.pos_y++;
			printk_info.pos_x = 0;
		}
		if(printk_info.pos_y >= (printk_info.screen_y / printk_info.csize_y))
		{
			printk_info.pos_y = 0;
		}

	}
	return i;
}

