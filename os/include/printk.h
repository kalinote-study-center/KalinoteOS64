/* 内核打印相关功能，用于非图形界面的模式 */

#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <fonts.h>
#include <types.h>
#include <linkage.h>
#include <graphic.h>
#include <stdarg.h>

#define is_digit(c)	((c) >= '0' && (c) <= '9')
#define do_div(n,base) ({ \
int __res; \
__asm__("divq %%rcx":"=a" (n),"=d" (__res):"0" (n),"1" (0),"c" (base)); \
__res; })

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

/* 该结构体用于记录内核打印时的相关信息 */
struct PRINTK_INFO {
	int mode;					/* 系统画面模式(0 - 纯文字命令行、1 - 图形界面) */
	int screen_x,screen_y;		/* 屏幕分辨率 */
	int pos_x,pos_y;			/* 光标位置 */
	int csize_x,csize_y;		/* 字符大小(像素) */
	vram_addr *buf;				/* 图像帧缓冲区 */
	unsigned long buf_length;	/* 图像帧缓冲区长度 */
} printk_info;

void putchar(vram_addr *vram, int xsize, int x, int y,
	color_code FColor, color_code BColor, font_type font);			/* 在指定位置绘制一个字符 */
void putchars_asc(vram_addr *vram, int xsize, int x, int y,
	color_code FColor, color_code BColor, font_type *s);			/* 在指定位置绘制一个字符串 */
static char * number(char * str, long num, int base, int size, int precision ,int type);
int vsprintf(char * buf,const char *fmt, va_list args);

#endif
