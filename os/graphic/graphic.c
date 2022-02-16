/* 图形绘制相关 */
#include <graphic.h>
#include <printk.h>
#include <types.h>
#include <lib.h>
#include <layer.h>
#include <memory.h>
#include <asm.h>

void boxfill(vram_addr *vram, int xsize, color_code color, int x0, int y0, int x1, int y1) {
	/* 绘制方块 */
	/* 需要注意的是，这里的x1和y1是对角点的坐标，而不是边长 */
	int x, y;
	
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = color;
	}
	
}

void init_screen(vram_addr *vram, int x, int y) {
	/* 桌面图形初始化，进入图形模式 */
	printk_info.mode = 1;		/* 图形模式 */
	struct Page *background_page = NULL;
	struct Page *taskbar_page = NULL;
	
	/* 图形层缓冲区内存页设置 */
	background_page = alloc_pages(ZONE_NORMAL, ((x*y*4)/2097152) + 1, 0);
	taskbar_page = alloc_pages(ZONE_NORMAL, 1, 0);
	page_init(background_page, PG_Kernel);
	page_init(taskbar_page, PG_Kernel);
	layer_background_buf = (vram_addr *)Phy_To_Virt(background_page->PHY_address);
	layer_taskbar_buf = (vram_addr *)Phy_To_Virt(taskbar_page->PHY_address);
	
	/* 初始化图层管理器 */
	base_layer_controller = layer_controller_init(0);

	/* 纯色底图背景 */
	layer_background = layer_alloc(base_layer_controller);
	layer_setbuf(layer_background, layer_background_buf, x, y, INTRANSPARENT);
	layer_background->flags = LAYER_BACK;
	boxfill(layer_background->buf, layer_background->bxsize, LDBLUE, 0, 0, layer_background->bxsize, layer_background->bysize);
	layer_move(layer_background, 0, 0);
	layer_set_height(layer_background, 1);
	
	/* 绘制taskbar(暂时，等后面做了菜单和按钮之类的功能以后再分离出来) */
	layer_taskbar = layer_alloc(base_layer_controller);
	layer_setbuf(layer_taskbar, layer_taskbar_buf, x, 28, INTRANSPARENT);
	layer_taskbar->flags = LAYER_TASKBAR;
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, BGREY,  0,     layer_taskbar->bysize - 28, layer_taskbar->bxsize -  1, layer_taskbar->bysize - 28);
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, WHITE,  0,     layer_taskbar->bysize - 27, layer_taskbar->bxsize -  1, layer_taskbar->bysize - 27);
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, BGREY,  0,     layer_taskbar->bysize - 26, layer_taskbar->bxsize -  1, layer_taskbar->bysize -  1);

	/* 按钮 */
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, WHITE,  3,     layer_taskbar->bysize - 24, 74,     layer_taskbar->bysize - 24);			/* 白色上横线 */
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, WHITE,  2,     layer_taskbar->bysize - 24,  2,     layer_taskbar->bysize -  4);			/* 白色左竖线 */
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, DGREY,  3,     layer_taskbar->bysize -  4, 74,     layer_taskbar->bysize -  4);			/* 灰色右竖线 */
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, DGREY, 74,     layer_taskbar->bysize - 23, 74,     layer_taskbar->bysize -  5);			/* 灰色下横线 */
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, BLACK,  2,     layer_taskbar->bysize -  3, 74,     layer_taskbar->bysize -  3);
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, BLACK, 75,     layer_taskbar->bysize - 24, 75,     layer_taskbar->bysize -  3);
	
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, DGREY, layer_taskbar->bxsize - 75, layer_taskbar->bysize - 24, layer_taskbar->bxsize -  4, layer_taskbar->bysize - 24);
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, DGREY, layer_taskbar->bxsize - 75, layer_taskbar->bysize - 23, layer_taskbar->bxsize - 75, layer_taskbar->bysize -  4);
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, WHITE, layer_taskbar->bxsize - 75, layer_taskbar->bysize -  3, layer_taskbar->bxsize -  4, layer_taskbar->bysize -  3);
	boxfill(layer_taskbar->buf, layer_taskbar->bxsize, WHITE, layer_taskbar->bxsize -  3, layer_taskbar->bysize - 24, layer_taskbar->bxsize -  3, layer_taskbar->bysize -  3);
	layer_move(layer_taskbar, 0, printk_info.screen_y - 28);
	layer_set_height(layer_taskbar, 3);

	/* 鼠标指针 */
	layer_mouse = layer_alloc(base_layer_controller);
	layer_setbuf(layer_mouse, mouse_buf, MOUSE_SIDELENGTH, MOUSE_SIDELENGTH, TRANSPARENT);
	layer_mouse->flags = LAYER_MOUSE;
	set_mouse_cursor(layer_mouse->buf, TRANSPARENT, CURSOR_TYPE_NORMAL);
	mouse_cursor.x = printk_info.screen_x / 2;
	mouse_cursor.y = printk_info.screen_y / 2;
	layer_move(layer_mouse, printk_info.screen_x / 2, printk_info.screen_y / 2);
	layer_set_height(layer_mouse, base_layer_controller->top + 1);
}

void set_mouse_cursor(vram_addr *mouse, color_code bc, int type){
	/* 准备鼠标指针 */
	/* 
	* 在这个函数中，vram_addr *mouse为鼠标指针的图形缓冲区，
	* color_code bc是鼠标指针背景色，type是鼠标指针种类
	* 对于绘制光标(将光标内容填充到vram中)的思路：
	* 每一行进行绘制，如果结构体中的x(行)小于MOUSE_SIDELENGTH常量，则以bc进行补齐
	*/
	int x, y;
	struct cursor cur;
	
	switch(type) {
		default:
		case CURSOR_TYPE_NORMAL:
			/* 普通光标 */
			cur = cursor_normal;
			break;
		case CURSOR_TYPE_TEXT_SELECTION:
			/* 文字选择符 */
			cur = cursor_text_selection;
			break;
	}

	for(y = 0; y < cur.y; y++) {
		for(x = 0; x < cur.x; x++) {
			if(cur.img[y*cur.x+x] == '*')
				mouse[y*MOUSE_SIDELENGTH+x] = BLACK;
			if(cur.img[y*cur.x+x] == 'O')
				mouse[y*MOUSE_SIDELENGTH+x] = WHITE;
			if(cur.img[y*cur.x+x] == '.')
				mouse[y*MOUSE_SIDELENGTH+x] = bc;
		}
		for(x = cur.x; x < MOUSE_SIDELENGTH; x++)		/* 补齐x */
			mouse[y*MOUSE_SIDELENGTH+x] = bc;
	}
	if(cur.y < MOUSE_SIDELENGTH)		/* 补齐y */
		for(x = cur.y * MOUSE_SIDELENGTH; x < MOUSE_SIDELENGTH * MOUSE_SIDELENGTH; x++)
			mouse[x] = bc;
}

void mouse_cursor_move(int x, int y) {
	/* 坐标限制，判断是否出界 */
	if(x >= printk_info.screen_x)
		x = printk_info.screen_x;
	if(y >= printk_info.screen_y)
		y = printk_info.screen_y;
	if(x < 0)
		x = 0;
	if(y < 0)
		y = 0;
	
	mouse_cursor.x = x;
	mouse_cursor.y = y;
	layer_move(layer_mouse, mouse_cursor.x, mouse_cursor.y);
}
