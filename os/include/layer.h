/* 图层管理 */

#ifndef __LAYER_H__
#define __LAYER_H__

#include <types.h>
#include <graphic.h>

#define MAX_LAYERS	64

/* flags */
#define LAYER_NO_USE		0				// 未被使用
#define LAYER_USEING		1				// 正在被使用，且未设定类型
#define LAYER_APIWIN		2				// 外部API窗口
#define LAYER_CONS			3				// 命令行窗口
#define LAYER_NO_TITLE		4				// 其他无标题栏窗口
#define LAYER_DEBUG_CONS	5				// DEBUG命令行窗口
#define	LAYER_BACK			101				// 背景层
#define	LAYER_TASKBAR		102				// 任务栏层
#define	LAYER_MOUSE			103				// 鼠标指针层
#define	LAYER_MENU			104				// 菜单栏层
#define	LAYER_BUTTON		105				// 按钮层

struct layer {
	/* 图层结构 */
	vram_addr *buf;							/* 图像内容 */
	int bxsize, bysize, vx0, vy0;			/* bxsize和bysize是图像整体大小，vx0和vy0是图层在父图层画面上的坐标位置(v是vram的省略) */
	int col_inv, height, flags;				/* col_inv是透明色色号，height表示图层高度，flags表示图层设定信息 */
	struct layer_controller *layer_ctl;		/* 所属图层管理器 */
	struct layer_controller *sub_layer_ctl;	/* 子图层管理器 */
};

struct layer_controller {
	/* 图层管理 */
	vram_addr *vram, *map;						/* 图形帧缓冲区 */
	int xsize, ysize, top;						/* xsize和ysize图层大小，top是最顶层图层(鼠标指针层)高度 */
	struct layer *f_layer;						/* 父图层(如果有的话) */
	struct layer *layers_addr[MAX_LAYERS];		/* 通过图层高度升序来存放所有图层的地址 */
	struct layer layers[MAX_LAYERS];			/* 存放所有图层数据结构 */
};


struct layer_controller *layer_controller_init(struct layer *f_layer);						/* 初始化图层管理系统 */
struct layer *layer_alloc(struct layer_controller *lay_ctl);								/* 从图层管理器中获取一个图层 */
void layer_setbuf(struct layer *lay, vram_addr *buf, int xsize, int ysize, int col_inv);	/* 设置图层缓冲区信息 */
void layer_set_height(struct layer *lay, int height);										/* 设置图层高度并进行绘制 */
void layer_refresh(struct layer *lay, int bx0, int by0, int bx1, int by1);					/* 刷新图层 */
void layer_refresh_area(struct layer_controller *layer_ctl,
	int vx0, int vy0, int vx1, int vy1, int h0, int h1);									/* 区域刷新 */
void layer_refreshmap(struct layer_controller *layer_ctl,
	int vx0, int vy0, int vx1, int vy1, int h0);											/* 向map中进行预刷新 */
void layer_move(struct layer *lay, int vx0, int vy0);										/* 上下左右移动图层 */

/*
* 在默认情况下，base layer的每一层分别为：
* 0:文字命令模式界面
* 1:纯色底层背景
* 2:桌面壁纸(如果有)
* 3:taskbar
* 4~n:窗口等
* n+1:鼠标指针(始终为top)
*/
struct layer_controller *base_layer_controller;					/* 系统最底层的图层管理器 */
struct layer *layer_mouse;										/* 鼠标图层 */
struct layer *layer_background;									/* 底层图层的背景图层(一般是桌面壁纸) */
struct layer *layer_taskbar;									/* 任务栏图层 */

/* 相关图层缓冲区 */
vram_addr mouse_buf[MOUSE_SIDELENGTH*MOUSE_SIDELENGTH] = {0};	/* 鼠标图形缓冲区 */
vram_addr *layer_background_buf;								/* 桌面图形缓冲区 */
vram_addr *layer_taskbar_buf;									/* 任务栏图形缓冲区 */

#endif
