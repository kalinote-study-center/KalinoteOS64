/* 图形绘制相关 */

#ifndef __GRAPHIC_H__
#define __GRAPHIC_H__

#include <types.h>

#define WHITE 			0x00ffffff		// 白
#define BLACK 			0x00000000		// 黑
#define RED				0x00ff0000		// 红
#define ORANGE			0x00ff8000		// 橙
#define YELLOW			0x00ffff00		// 黄
#define GREEN			0x0000ff00		// 绿
#define BLUE			0x000000ff		// 蓝
#define INDIGO			0x0000ffff		// 靛
#define PURPLE			0x008000ff		// 紫
#define BGREY			0x00c6c6c6		// 亮灰
#define DRED			0x00840000      // 深红
#define DGREEN			0x00008400      // 深绿
#define DYELLOW			0x00848400      // 深黄
#define DBLUE			0x00000084      // 深蓝
#define DPURPLE			0x00840084      // 深紫
#define LDBLUE			0x00008484      // 深靛
#define DGREY			0x00848484      // 深灰
#define TRANSPARENT		0xff000000		// 透明
#define INTRANSPARENT	0xffffffff		// 非透明

struct mouse_cursor {
	unsigned int x, y;
} mouse_cursor;

void boxfill(vram_addr *vram, int xsize, color_code color, int x0, int y0, int x1, int y1);		// 绘制方块
void init_screen(vram_addr *vram, int x, int y);												// 桌面图形初始化，进入图形模式
void set_mouse_cursor(vram_addr *mouse, color_code bc, int type);									// 设置鼠标指针图形
void mouse_cursor_move(int x, int y);															// 移动鼠标指针

/* 鼠标指针 */
#define MOUSE_SIDELENGTH			32	// 鼠标指针缓冲区边长大小
#define CURSOR_TYPE_NORMAL			0	/* 普通光标 */
#define CURSOR_TYPE_TEXT_SELECTION	1	/* 文字选择符 */
struct cursor {
	/* 该结构体用于构造鼠标指针图像 */
	int x,y;		/* 鼠标指针图像的高宽 */
	char *img;		/* 该指针指向对应的鼠标指针内容数组 */
};

/* 光标 */
static char cursor_normal_img[16][10] = {
	"*.........",
	"**........",
	"*O*.......",
	"*OO*......",
	"*OOO*.....",
	"*OOOO*....",
	"*OOOOO*...",
	"*OOOOOO*..",
	"*OOOOOOO*.",
	"*OOOO*****",
	"*OO*O*....",
	"*O*.*O*...",
	"**..*O*...",
	"*....*O*..",
	".....*O*..",
	"......*..."
};
static char cursor_text_selection_img[16][7] = {
	"***.***",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"...*...",
	"***.***"
};
struct cursor cursor_normal = {
	.x = 10, 
	.y = 16,
	.img = &(cursor_normal_img[0][0])};							/* 普通光标结构 */
struct cursor cursor_text_selection = {
	.x = 7,
	.y = 16,
	.img = &(cursor_text_selection_img[0][0])};					/* 字符选择光标结构 */

#endif
