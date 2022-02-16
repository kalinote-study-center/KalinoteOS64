/* 图层管理 */
#include <layer.h>
#include <types.h>
#include <memory.h>

struct layer_controller *layer_controller_init(struct layer *f_layer) {
	/* 初始化图层管理系统 */
	struct layer_controller *layer_ctl;
	int i;
	struct Page *map_page = NULL;
	
	layer_ctl = (struct layer_controller *)kmalloc(sizeof(struct layer_controller), 0);
	if(!layer_ctl)		/* 内存分配失败 */
		goto ret;
	if(!f_layer)
		map_page = alloc_pages(ZONE_NORMAL, ((printk_info.screen_x*printk_info.screen_y*4)/2097152) + 1, 0);
	else
		map_page = alloc_pages(ZONE_NORMAL, ((f_layer->bxsize*f_layer->bysize*4)/2097152) + 1, 0);
	page_init(map_page, PG_Kernel);
	layer_ctl->map = (vram_addr *)Phy_To_Virt(map_page->PHY_address);
	if(!layer_ctl->map) {
		kfree(layer_ctl);
		layer_ctl = 0;
		goto ret;
	}
	if(!f_layer) {
		/* base_layer_controller */
		layer_ctl->vram = printk_info.buf;
		layer_ctl->xsize = printk_info.screen_x;
		layer_ctl->ysize = printk_info.screen_y;
	} else {
		layer_ctl->vram = f_layer->buf;
		layer_ctl->xsize = f_layer->bxsize;
		layer_ctl->ysize = f_layer->bysize;
	}
	layer_ctl->top = -1;			/* 没有图层 */
	layer_ctl->f_layer = f_layer;

	/* 初始化图层管理器中的图层数据结构 */
	for(i = 0; i < MAX_LAYERS; i++)
		layer_ctl->layers[i].flags = LAYER_NO_USE;		/* 将图层全部标记为未使用状态 */

ret:
	return layer_ctl;
}

struct layer *layer_alloc(struct layer_controller *layer_ctl) {
	/* 从图层管理器中获取一个图层 */
	struct layer *lay;
	int i;
	
	for(i = 0; i < MAX_LAYERS; i++) {
		/* 搜索一个没有被使用的数据结构 */
		if(layer_ctl->layers[i].flags == LAYER_NO_USE) {
			lay = &layer_ctl->layers[i];
			lay->flags = LAYER_USEING;
			lay->height = -1;
			lay->layer_ctl = layer_ctl;
			lay->sub_layer_ctl = 0;		/* 子图层控制器，需要时使用layer_controller_init进行初始化设置 */
			return lay;
		}
	}
	
	return 0;
}

void layer_setbuf(struct layer *lay, vram_addr *buf, int xsize, int ysize, int col_inv) {
	/* 设置图层缓冲区信息 */
	lay->buf = buf;
	lay->bxsize = xsize;
	lay->bysize = ysize;
	lay->col_inv = col_inv;
}

void layer_set_height(struct layer *lay, int height) {
	/* 设置图层高度并进行绘制 */
	int h, old = lay->height;
	struct layer_controller *layer_ctl = lay->layer_ctl;
	
	if(height > layer_ctl->top + 1)		/* 设置高度过高 */
		height = layer_ctl->top + 1;
	if(height < -1)		/* 设置高度过低 */
		height = -1;
	lay->height = height;
	
	/* 对layers按照高度进行重新排列 */
	if(old > height) {
		/* 比以前的高度更低 */
		if(height >= 0) {
			/* 把中间的向上移动 */
			for(h = old; h > height; h--) {
				layer_ctl->layers_addr[h] = layer_ctl->layers_addr[h - 1];
				layer_ctl->layers_addr[h]->height = h;
			}
			layer_ctl->layers_addr[height] = lay;
			layer_refreshmap(layer_ctl, lay->vx0, lay->vy0, lay->vx0 + lay->bxsize, lay->vy0 + lay->bysize, height + 1);
			layer_refresh_area(layer_ctl, lay->vx0, lay->vy0, lay->vx0 + lay->bxsize, lay->vy0 + lay->bysize, height + 1, old);
		} else {
			/* 隐藏 */
			if (layer_ctl->top > old) {
				/* 把上面的降下来 */
				for (h = old; h < layer_ctl->top; h++) {
					layer_ctl->layers_addr[h] = layer_ctl->layers_addr[h + 1];
					layer_ctl->layers_addr[h]->height = h;
				}
			}
			layer_ctl->top--;
			layer_refreshmap(layer_ctl, lay->vx0, lay->vy0, lay->vx0 + lay->bxsize, lay->vy0 + lay->bysize, 0);
			layer_refresh_area(layer_ctl, lay->vx0, lay->vy0, lay->vx0 + lay->bxsize, lay->vy0 + lay->bysize, 0, old - 1);
		}
	} else if (old < height) {
		/* 比以前的高度更高 */
		if (old >= 0) {
			/* 将中间的高度降低 */
			for (h = old; h < height; h++) {
				layer_ctl->layers_addr[h] = layer_ctl->layers_addr[h + 1];
				layer_ctl->layers_addr[h]->height = h;
			}
			layer_ctl->layers_addr[height] = lay;
		} else {	/* 由隐藏转为显示 */
			/* 将已经在上面的图层向上移动 */
			for (h = layer_ctl->top; h >= height; h--) {
				layer_ctl->layers_addr[h + 1] = layer_ctl->layers_addr[h];
				layer_ctl->layers_addr[h + 1]->height = h + 1;
			}
			layer_ctl->layers_addr[height] = lay;
			layer_ctl->top++; /* 顶部高度增加 */
		}
		layer_refreshmap(layer_ctl, lay->vx0, lay->vy0, lay->vx0 + lay->bxsize, lay->vy0 + lay->bysize, height);
		layer_refresh_area(layer_ctl, lay->vx0, lay->vy0, lay->vx0 + lay->bxsize, lay->vy0 + lay->bysize, height, height);
	}
}

void layer_refresh(struct layer *lay, int bx0, int by0, int bx1, int by1) {
	/* 刷新图层 */
	if(lay->height >= 0)
		layer_refresh_area(lay->layer_ctl, lay->vx0 + bx0, lay->vy0 + by0, lay->vx0 + bx1, lay->vy0 + by1, lay->height, lay->height);
}

void layer_refresh_area(struct layer_controller *layer_ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1) {
	/* 区域刷新 */
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	vram_addr *buf, *vram = layer_ctl->vram, *map = layer_ctl->map, sid;
	struct layer *lay;
	/* 不对画面外的图像进行刷新 */
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > layer_ctl->xsize) { vx1 = layer_ctl->xsize; }
	if (vy1 > layer_ctl->ysize) { vy1 = layer_ctl->ysize; }
	for (h = h0; h <= h1; h++) {
		lay = layer_ctl->layers_addr[h];
		buf = lay->buf;
		sid = lay - layer_ctl->layers;
		bx0 = vx0 - lay->vx0;
		by0 = vy0 - lay->vy0;
		bx1 = vx1 - lay->vx0;
		by1 = vy1 - lay->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > lay->bxsize) { bx1 = lay->bxsize; }
		if (by1 > lay->bysize) { by1 = lay->bysize; }
		for (by = by0; by < by1; by++) {
			vy = lay->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = lay->vx0 + bx;
				if (map[vy * layer_ctl->xsize + vx] == sid) {
					vram[vy * layer_ctl->xsize + vx] = buf[by * lay->bxsize + bx];
				}
			}
		}
	}
}

void layer_move(struct layer *lay, int vx0, int vy0) {
	/* 上下左右移动图层 */
	int old_vx0 = lay->vx0, old_vy0 = lay->vy0;
	lay->vx0 = vx0;
	lay->vy0 = vy0;
	if (lay->height >= 0) {		/* 如果正在显示，则进行刷新 */
		layer_refreshmap(lay->layer_ctl, old_vx0, old_vy0, old_vx0 + lay->bxsize, old_vy0 + lay->bysize, 0);
		layer_refreshmap(lay->layer_ctl, vx0, vy0, vx0 + lay->bxsize, vy0 + lay->bysize, lay->height);
		layer_refresh_area(lay->layer_ctl, old_vx0, old_vy0, old_vx0 + lay->bxsize, old_vy0 + lay->bysize, 0, lay->height - 1);
		layer_refresh_area(lay->layer_ctl, vx0, vy0, vx0 + lay->bxsize, vy0 + lay->bysize, lay->height, lay->height);
	}
}

void layer_refreshmap(struct layer_controller *layer_ctl, int vx0, int vy0, int vx1, int vy1, int h0) {
	/* 向map中进行预刷新 */
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	vram_addr *buf, sid, *map = layer_ctl->map;
	struct layer *lay;
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > layer_ctl->xsize) { vx1 = layer_ctl->xsize; }
	if (vy1 > layer_ctl->ysize) { vy1 = layer_ctl->ysize; }
	
	for (h = h0; h <= layer_ctl->top; h++) {
		lay = layer_ctl->layers_addr[h];		/* 这里的lay可能会出现0 */
		sid = lay - layer_ctl->layers;
		buf = lay->buf;
		bx0 = vx0 - lay->vx0;
		by0 = vy0 - lay->vy0;
		bx1 = vx1 - lay->vx0;
		by1 = vy1 - lay->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > lay->bxsize) { bx1 = lay->bxsize; }
		if (by1 > lay->bysize) { by1 = lay->bysize; }
		for (by = by0; by < by1; by++) {
			vy = lay->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = lay->vx0 + bx;
				if (buf[by * lay->bxsize + bx] != lay->col_inv) {
					map[vy * layer_ctl->xsize + vx] = sid;
				}
			}
		}
	}
}

void layer_free(struct layer *lay) {
	/* 释放图层 */
	if (lay->height >= 0)		/* 对显示中的图层进行隐藏 */
		layer_set_height(lay, -1);
	lay->flags = LAYER_NO_USE;	/* 标记为未使用 */
}
