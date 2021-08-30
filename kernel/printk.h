/* 内核打印 */
#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <stdarg.h>
#include "font.h"
#include "linkage.h"

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

#define COL_WHITE 	0x00ffffff		//白
#define COL_BLACK 	0x00000000		//黑
#define COL_RED		0x00ff0000		//红
#define COL_ORANGE	0x00ff8000		//橙
#define COL_YELLOW	0x00ffff00		//黄
#define COL_GREEN	0x0000ff00		//绿
#define COL_BLUE	0x000000ff		//蓝
#define COL_INDIGO	0x0000ffff		//靛
#define COL_PURPLE	0x008000ff		//紫

#define is_digit(c)	((c) >= '0' && (c) <= '9')
#define do_div(n,base) ({ \
int __res; \
__asm__("divq %%rcx":"=a" (n),"=d" (__res):"0" (n),"1" (0),"c" (base)); \
__res; })

extern unsigned char font_ascii[256][16];		/* ascii字符像素映射(font.h) */

char buf[4096]={0};

struct position {
	/* 屏幕信息结构体 */
	/* 分辨率 */
	int XResolution;
	int YResolution;
	/* 位置 */
	int XPosition;
	int YPosition;
	/* 字符尺寸 */
	int XCharSize;
	int YCharSize;
	/* 帧缓存起始地址及容量 */
	unsigned int * FB_addr;
	unsigned long FB_length;
}Pos;

void putchar(unsigned int * fb,int Xsize,int x,int y,
	unsigned int FRcolor,unsigned int BKcolor,unsigned char font);
int skip_atoi(const char **s);
int vsprintf(char * buf,const char *fmt, va_list args);
int color_printk(unsigned int FRcolor,unsigned int BKcolor,const char * fmt,...);		/* 彩色内核打印 */

#endif
