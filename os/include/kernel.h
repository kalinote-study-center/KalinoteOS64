/* 内核相关结构和信息 */

#ifndef __KERNEL_H__
#define __KERNEL_H__

/* platform */
#define PLATFORM_HYPER_V	0
#define PLATFORM_BOCHS		1
struct PLATFORM {int platform;} platform;

#endif
