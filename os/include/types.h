/* 这里定义了一些数据类型 */
/* 之所以定义这些类型，是为了规范化操作系统的类型，避免不同函数的相同(类似)变量使用不同类型 */

#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned int vram_addr;			/* 图像缓冲区地址类型 */
typedef unsigned int color_code;		/* 颜色码类型 */
typedef unsigned char font_type;		/* 字符数据类型 */
typedef unsigned char mem_addr8;		/* 内存地址(指针) */
typedef unsigned short mem_addr16;		/* 内存地址(指针) */
typedef unsigned int mem_addr32;		/* 内存地址(指针) */
typedef unsigned long mem_addr64;		/* 内存地址(指针) */

#endif
