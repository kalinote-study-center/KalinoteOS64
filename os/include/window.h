/* 窗口绘制与管理 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <types.h>

struct wintheme {
	/* 窗口色彩主题和其他可视化相关信息 */
	color_code activate, deactivate, bg_color;		// 聚焦、未聚焦、背景颜色
};

struct window {
	struct layer_controller *layer_ctl;		/* 每一个窗口对应一个单独的layer_controller，隶属于base_layer_controller或其他layer_controller */
	char *wtitle;							/* 窗口名称 */
	int xsize, ysize;						/* 窗口大小，需要与layer_ctl中的xsize和ysize保持一致 */
	struct wintheme wtheme;					/* 窗口主题 */
};

#endif
