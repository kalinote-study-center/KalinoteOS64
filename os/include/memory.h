/* 内存管理 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <printk.h>
#include <lib.h>

#define PTRS_PER_PAGE	512			/* 每个单元 8 字节  */
#define PAGE_OFFSET	((unsigned long)0xffff800000000000)
#define PAGE_GDT_SHIFT	39
#define PAGE_1G_SHIFT	30
#define PAGE_2M_SHIFT	21
#define PAGE_4K_SHIFT	12
#define PAGE_2M_SIZE	(1UL << PAGE_2M_SHIFT)
#define PAGE_4K_SIZE	(1UL << PAGE_4K_SHIFT)
#define PAGE_2M_MASK	(~ (PAGE_2M_SIZE - 1))
#define PAGE_4K_MASK	(~ (PAGE_4K_SIZE - 1))
#define PAGE_2M_ALIGN(addr)	(((unsigned long)(addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK)
#define PAGE_4K_ALIGN(addr)	(((unsigned long)(addr) + PAGE_4K_SIZE - 1) & PAGE_4K_MASK)
#define Virt_To_Phy(addr)	((unsigned long)(addr) - PAGE_OFFSET)
#define Phy_To_Virt(addr)	((unsigned long *)((unsigned long)(addr) + PAGE_OFFSET))
#define Virt_To_2M_Page(kaddr)	(memory_management_struct.pages_struct + (Virt_To_Phy(kaddr) >> PAGE_2M_SHIFT))
#define Phy_to_2M_Page(kaddr)	(memory_management_struct.pages_struct + ((unsigned long)(kaddr) >> PAGE_2M_SHIFT))

/* 页表属性 */
#define PAGE_XD		(unsigned long)0x1000000000000000			// 第63位 执行禁用
#define	PAGE_PAT	(unsigned long)0x1000						// 第12位 页属性表 
#define	PAGE_Global	(unsigned long)0x0100						// 第8位 全局页面：1，全局；0，局部
#define	PAGE_PS		(unsigned long)0x0080						// 第7位 页大小：1，大页；0，小页； 
#define	PAGE_Dirty	(unsigned long)0x0040						// 第6位 脏数据：1，脏数据；0，干净
#define	PAGE_Accessed	(unsigned long)0x0020					// 第5位 已访问：1，已访问；0，未访问； 
#define PAGE_PCD	(unsigned long)0x0010						// 第4位 页面级缓存禁用
#define PAGE_PWT	(unsigned long)0x0008						// 第3位 页级直写
#define	PAGE_U_S	(unsigned long)0x0004						// 第2位 用户和系统级，1用户和系统；0，系统
#define	PAGE_R_W	(unsigned long)0x0002						// 第1位 读写：1，读写；0，读； 
#define	PAGE_Present	(unsigned long)0x0001					// 第0位 是否存在：1，存在；0，不存在；
#define PAGE_KERNEL_GDT		(PAGE_R_W | PAGE_Present)							//1,0
#define PAGE_KERNEL_Dir		(PAGE_R_W | PAGE_Present)							//1,0
#define	PAGE_KERNEL_Page	(PAGE_PS  | PAGE_R_W | PAGE_Present)				//7,1,0
#define PAGE_USER_GDT		(PAGE_U_S | PAGE_R_W | PAGE_Present)				//1,0
#define PAGE_USER_Dir		(PAGE_U_S | PAGE_R_W | PAGE_Present)				//2,1,0
#define	PAGE_USER_Page		(PAGE_PS  | PAGE_U_S | PAGE_R_W | PAGE_Present)		//7,2,1,0	

typedef struct {unsigned long pml4t;} pml4t_t;
#define	mk_mpl4t(addr,attr)	((unsigned long)(addr) | (unsigned long)(attr))
#define set_mpl4t(mpl4tptr,mpl4tval)	(*(mpl4tptr) = (mpl4tval))
typedef struct {unsigned long pdpt;} pdpt_t;
#define mk_pdpt(addr,attr)	((unsigned long)(addr) | (unsigned long)(attr))
#define set_pdpt(pdptptr,pdptval)	(*(pdptptr) = (pdptval))
typedef struct {unsigned long pdt;} pdt_t;
#define mk_pdt(addr,attr)	((unsigned long)(addr) | (unsigned long)(attr))
#define set_pdt(pdtptr,pdtval)		(*(pdtptr) = (pdtval))
typedef struct {unsigned long pt;} pt_t;
#define mk_pt(addr,attr)	((unsigned long)(addr) | (unsigned long)(attr))
#define set_pt(ptptr,ptval)		(*(ptptr) = (ptval))
/* 所有页面区域选择 */
#define ZONE_DMA	(1 << 0)
#define ZONE_NORMAL	(1 << 1)
#define ZONE_UNMAPED	(1 << 2)
/* 页面属性表 */
#define PG_PTable_Maped	(1 << 0)	 // 是否在页表中映射
#define PG_Kernel_Init	(1 << 1)     // 是否为内核初始化程序
#define PG_Device	(1 << 2)         // 设备寄存器/内存(置位)或物理内存空间(复位)
#define PG_Kernel	(1 << 3)         // 内核层地址(置位)/应用层地址(复位)
#define PG_Shared	(1 << 4)         // 内存页是否已被共享

/* 结构页面属性(alloc_页面标志) */
#define PG_Referenced	(1 << 2)
#define PG_Dirty	(1 << 3)
#define PG_Active	(1 << 4)
#define PG_Up_To_Date	(1 << 5)
#define PG_K_Share_To_U	(1 << 8)
#define PG_Slab		(1 << 9)
struct Zone
{
	struct Page * 	pages_group;
	unsigned long	pages_length;
	
	unsigned long	zone_start_address;
	unsigned long	zone_end_address;
	unsigned long	zone_length;
	unsigned long	attribute;

	struct Global_Memory_Descriptor * GMD_struct;

	unsigned long	page_using_count;
	unsigned long	page_free_count;

	unsigned long	total_pages_link;
};
struct Page {
	struct Zone *	zone_struct;		/* 指向本页所属区域结构体 */
	unsigned long	PHY_address;		/* 页的物理地址 */
	unsigned long	attribute;			/* 页的属性 */
	unsigned long	reference_count;	/* 描述该页引用次数 */
	unsigned long	age;				/* 描述该页创建时间 */
};
int ZONE_DMA_INDEX	= 0;
int ZONE_NORMAL_INDEX	= 0;	//low 1GB RAM ,was mapped in pagetable
int ZONE_UNMAPED_INDEX	= 0;	//above 1GB RAM,unmapped in pagetable

#define MAX_NR_ZONES	10	//max zone

unsigned long * Global_CR3 = NULL;

struct E820 {
	unsigned long address;
	unsigned long length;
	unsigned int	type;
}__attribute__((packed));
struct Global_Memory_Descriptor {
	struct E820 	e820[32];
	unsigned long 	e820_length;

	unsigned long * bits_map;
	unsigned long 	bits_size;
	unsigned long   bits_length;

	struct Page *	pages_struct;
	unsigned long	pages_size;
	unsigned long 	pages_length;

	struct Zone * 	zones_struct;
	unsigned long	zones_size;
	unsigned long 	zones_length;

	unsigned long 	start_code , end_code , end_data , end_brk;

	unsigned long	end_of_struct;	
};

extern struct Global_Memory_Descriptor memory_management_struct;
unsigned long page_init(struct Page * page,unsigned long flags);
unsigned long page_clean(struct Page * page);
void init_memory();			/* 初始化内存信息 */
struct Page * alloc_pages(int zone_select,int number,unsigned long page_flags);

#define	flush_tlb_one(addr)	\
	__asm__ __volatile__	("invlpg	(%0)	\n\t"::"r"(addr):"memory")
#define flush_tlb()						\
do								\
{								\
	unsigned long	tmpreg;					\
	__asm__ __volatile__ 	(				\
				"movq	%%cr3,	%0	\n\t"	\
				"movq	%0,	%%cr3	\n\t"	\
				:"=r"(tmpreg)			\
				:				\
				:"memory"			\
				);				\
}while(0)

static inline unsigned long * Get_gdt() {
	unsigned long * tmp;
	__asm__ __volatile__	(
					"movq	%%cr3,	%0	\n\t"
					:"=r"(tmp)
					:
					:"memory"
				);
	return tmp;
}

struct Slab {
	struct List list;
	struct Page * page;

	unsigned long using_count;
	unsigned long free_count;

	void * Vaddress;

	unsigned long color_length;
	unsigned long color_count;

	unsigned long * color_map;
};

struct Slab_cache {
	unsigned long	size;
	unsigned long	total_using;
	unsigned long	total_free;
	struct Slab *	cache_pool;
	struct Slab *	cache_dma_pool;
	void *(* constructor)(void * Vaddress,unsigned long arg);
	void *(* destructor)(void * Vaddress,unsigned long arg);
};

struct Slab_cache kmalloc_cache_size[16] =  {
	{32	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{64	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{128	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{256	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{512	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{1024	,0	,0	,NULL	,NULL	,NULL	,NULL},			//1KB
	{2048	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{4096	,0	,0	,NULL	,NULL	,NULL	,NULL},			//4KB
	{8192	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{16384	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{32768	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{65536	,0	,0	,NULL	,NULL	,NULL	,NULL},			//64KB
	{131072	,0	,0	,NULL	,NULL	,NULL	,NULL},			//128KB
	{262144	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{524288	,0	,0	,NULL	,NULL	,NULL	,NULL},
	{1048576,0	,0	,NULL	,NULL	,NULL	,NULL},			//1MB
};

#define SIZEOF_LONG_ALIGN(size) ((size + sizeof(long) - 1) & ~(sizeof(long) - 1) )
#define SIZEOF_INT_ALIGN(size) ((size + sizeof(int) - 1) & ~(sizeof(int) - 1) )

unsigned long get_page_attribute(struct Page * page);
unsigned long set_page_attribute(struct Page * page,unsigned long flags);
void * kmalloc(unsigned long size,unsigned long flags);
struct Slab * kmalloc_create(unsigned long size);
unsigned long kfree(void * address);
struct Slab_cache * slab_create(unsigned long size,void *(* constructor)(void * Vaddress,unsigned long arg),
	void *(* destructor)(void * Vaddress,unsigned long arg),unsigned long arg);
unsigned long slab_destroy(struct Slab_cache * slab_cache);
void * slab_malloc(struct Slab_cache * slab_cache,unsigned long arg);
unsigned long slab_free(struct Slab_cache * slab_cache,void * address,unsigned long arg);
unsigned long slab_init();



#endif
