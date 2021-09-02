/* 中断处理相关 */
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include "linkage.h"

void init_interrupt(void);
void do_IRQ(unsigned long regs,unsigned long nr);

#endif
