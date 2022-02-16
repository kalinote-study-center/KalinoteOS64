/* 这里定义了一些通用功能和宏 */

#ifndef __LIB_H__
#define __LIB_H__

#include <types.h>

#define NULL 0

#define container_of(ptr,type,member)							\
({											\
	typeof(((type *)0)->member) * p = (ptr);					\
	(type *)((unsigned long)p - (unsigned long)&(((type *)0)->member));		\
})

/* List数据结构 */
struct List {
	struct List * prev;
	struct List * next;
};

inline void list_init(struct List * list) {
	list->prev = list;
	list->next = list;
}

inline void list_add_to_behind(struct List * entry,struct List * new) {
	////add to entry behind
	new->next = entry->next;
	new->prev = entry;
	new->next->prev = new;
	entry->next = new;
}

inline void list_add_to_before(struct List * entry,struct List * new) {
	////add to entry behind
	new->next = entry;
	entry->prev->next = new;
	new->prev = entry->prev;
	entry->prev = new;
}

inline void list_del(struct List * entry) {
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

inline long list_is_empty(struct List * entry) {
	if(entry == entry->next && entry->prev == entry)
		return 1;
	else
		return 0;
}

inline struct List * list_prev(struct List * entry) {
	if(entry->prev != NULL)
		return entry->prev;
	else
		return NULL;
}

inline struct List * list_next(struct List * entry) {
	if(entry->next != NULL)
		return entry->next;
	else
		return NULL;
}

static inline int strlen(char * String) {
	register int __res;
	__asm__	__volatile__	(	"cld	\n\t"
					"repne	\n\t"
					"scasb	\n\t"
					"notl	%0	\n\t"
					"decl	%0	\n\t"
					:"=c"(__res)
					:"D"(String),"a"(0),"0"(0xffffffff)
					:
				);
	return __res;
}

inline void * memcpy(void *From,void * To,long Num) {
	int d0,d1,d2;
	__asm__ __volatile__	(	"cld	\n\t"
					"rep	\n\t"
					"movsq	\n\t"
					"testb	$4,%b4	\n\t"
					"je	1f	\n\t"
					"movsl	\n\t"
					"1:\ttestb	$2,%b4	\n\t"
					"je	2f	\n\t"
					"movsw	\n\t"
					"2:\ttestb	$1,%b4	\n\t"
					"je	3f	\n\t"
					"movsb	\n\t"
					"3:	\n\t"
					:"=&c"(d0),"=&D"(d1),"=&S"(d2)
					:"0"(Num/8),"q"(Num),"1"(To),"2"(From)
					:"memory"
				);
	return To;
}

inline int memcmp(void * FirstPart,void * SecondPart,long Count) {
	register int __res;

	__asm__	__volatile__	(	"cld	\n\t"		//clean direct
					"repe	\n\t"		//repeat if equal
					"cmpsb	\n\t"
					"je	1f	\n\t"
					"movl	$1,	%%eax	\n\t"
					"jl	1f	\n\t"
					"negl	%%eax	\n\t"
					"1:	\n\t"
					:"=a"(__res)
					:"0"(0),"D"(FirstPart),"S"(SecondPart),"c"(Count)
					:
				);
	return __res;
}

static inline void * memset(void * Address,unsigned char C,long Count) {
	int d0,d1;
	unsigned long tmp = C * 0x0101010101010101UL;
	__asm__	__volatile__	(	"cld	\n\t"
					"rep	\n\t"
					"stosq	\n\t"
					"testb	$4, %b3	\n\t"
					"je	1f	\n\t"
					"stosl	\n\t"
					"1:\ttestb	$2, %b3	\n\t"
					"je	2f\n\t"
					"stosw	\n\t"
					"2:\ttestb	$1, %b3	\n\t"
					"je	3f	\n\t"
					"stosb	\n\t"
					"3:	\n\t"
					:"=&c"(d0),"=&D"(d1)
					:"a"(tmp),"q"(Count),"0"(Count/8),"1"(Address)	
					:"memory"					
				);
	return Address;
}

inline unsigned long bit_set(unsigned long * addr,unsigned long nr) {
	return *addr | (1UL << nr);
}

inline unsigned long bit_get(unsigned long * addr,unsigned long nr) {
	return	*addr & (1UL << nr);
}


inline unsigned long bit_clean(unsigned long * addr,unsigned long nr) {
	return	*addr & (~(1UL << nr));
}

#endif
