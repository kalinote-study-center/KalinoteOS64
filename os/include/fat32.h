/* FAT32文件系统 */

#ifndef __FAT32_H__
#define __FAT32_H__

/* 文件属性 */
#define	ATTR_READ_ONLY	(1 << 0)		/* 只读 */
#define ATTR_HIDDEN	(1 << 1)            /* 隐藏文件 */
#define ATTR_SYSTEM	(1 << 2)            /* 系统文件 */
#define ATTR_VOLUME_ID	(1 << 3)        /* 卷标 */
#define ATTR_DIRECTORY	(1 << 4)        /* 目录 */
#define ATTR_ARCHIVE	(1 << 5)        /* 存档 */
#define ATTR_LONG_NAME	(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)		/* 长文件名(0x0f) */

struct Disk_Partition_Table_Entry {
	/* 硬盘分区表项 */
	unsigned char flags;
	unsigned char start_head;
	unsigned short  start_sector	:6,	//0~5
			start_cylinder	:10;	//6~15
	unsigned char type;
	unsigned char end_head;
	unsigned short  end_sector	:6,	//0~5
			end_cylinder	:10;	//6~15
	unsigned int start_LBA;
	unsigned int sectors_limit;
	
}__attribute__((packed));

struct Disk_Partition_Table {
	/* 硬盘分区表 */
	unsigned char BS_Reserved[446];
	struct Disk_Partition_Table_Entry DPTE[4];
	unsigned short BS_TrailSig;
}__attribute__((packed));

struct FAT32_BootSector {
	/* FAT32文件系统引导扇区 */
	/* 这个结构有一部分与FAT12(FAT16)相同 */
	unsigned char BS_jmpBoot[3];		/* 跳转指令 */
	unsigned char BS_OEMName[8];        /* 生产厂商名 */
	unsigned short BPB_BytesPerSec;     /* 每扇区字节数 */
	unsigned char BPB_SecPerClus;       /* 每簇扇区数 */
	unsigned short BPB_RsvdSecCnt;      /* 保留扇区数 */
	unsigned char BPB_NumFATs;          /* FAT表份数 */
	unsigned short BPB_RootEntCnt;      /* 根目录可容纳的目录项数 */
	unsigned short BPB_TotSec16;        /* 总扇区数 */
	unsigned char BPB_Media;            /* 介质描述符 */
	unsigned short BPB_FATSz16;         /* 每个FAT扇区数 */
	unsigned short BPB_SecPerTrk;       /* 每磁道扇区数 */
	unsigned short BPB_NumHeads;        /* 磁头数 */
	unsigned int BPB_HiddSec;           /* 隐藏扇区数 */
	unsigned int BPB_TotSec32;          /* 总扇区数(如果BPB_TotSec16为0则在此记录) */
	
	/* 下面是FAT32结构 */
	unsigned int BPB_FATSz32;           /* 每FAT扇区数(BPB_FATSz16需要为0) */
	unsigned short BPB_ExtFlags;        /* 扩展flag，其中bit0-3:活动FAT表; bit4-6:保留; bit7:FAT表更新模式; bit8-15:保留 */
	unsigned short BPB_FSVer;           /* 文件系统版本号 */
	unsigned int BPB_RootClus;          /* 根目录起始簇号 */
	unsigned short BPB_FSInfo;          /* FSInfo结构所在扇区号 */
	unsigned short BPB_BkBootSec;       /* 引导扇区备份扇区号 */
	unsigned char BPB_Reserved[12];     /* 保留 */
	/* 上面是FAT32结构 */

	unsigned char BS_DrvNum;            /* int 13h 驱动器号 */
	unsigned char BS_Reserved1;         /* 保留 */
	unsigned char BS_BootSig;           /* 扩展引导标记 */
	unsigned int BS_VolID;              /* 卷序列号 */
	unsigned char BS_VolLab[11];        /* 卷标 */
	unsigned char BS_FilSysType[8];     /* 文件系统类型 */

	unsigned char BootCode[420];        /* 引导代码 */

	unsigned short BS_TrailSig;         /* 结束标记0xaa55 */
}__attribute__((packed));

struct FAT32_FSInfo {
	/* FSInfo扇区结构 */
	unsigned int FSI_LeadSig;           /* FSInfo扇区标识符 */
	unsigned char FSI_Reserved1[480];   /* 保留 */
	unsigned int FSI_StrucSig;          /* 结构标识符 */
	unsigned int FSI_Free_Count;        /* 上一次记录的空闲簇大概数量 */
	unsigned int FSI_Nxt_Free;          /* 空闲簇的起始搜索位置 */
	unsigned char FSI_Reserved2[12];    /* 保留，置0 */
	unsigned int FSI_TrailSig;          /* 结束标识，0xaa550000 */
}__attribute__((packed));

struct FAT32_Directory {
	/* 短目录项结构(文件目录数据结构) */
	unsigned char DIR_Name[11];          /* 文件名和扩展名 */
	unsigned char DIR_Attr;              /* 文件属性 */
	unsigned char DIR_NTRes;             /* 微软保留 */
	unsigned char DIR_CrtTimeTenth;      /* 文件创建毫秒时间戳 */
	unsigned short DIR_CrtTime;	         /* 文件创建时间 */
	unsigned short DIR_CrtDate;          /* 文件创建日期 */
	unsigned short DIR_LastAccDate;      /* 最后访问日期 */
	unsigned short DIR_FstClusHI;        /* 起始簇号(高位) */
	unsigned short DIR_WrtTime;          /* 最后写入时间 */
	unsigned short DIR_WrtDate;          /* 最后写入日期 */
	unsigned short DIR_FstClusLO;        /* 起始簇号(低位) */
	unsigned int DIR_FileSize;           /* 文件大小 */
}__attribute__((packed));

#define LOWERCASE_BASE (8)
#define LOWERCASE_EXT (16)
struct FAT32_LongDirectory {
	/* 长目录项结构(文件目录数据结构) */
	unsigned char LDIR_Ord;              /* 序号 */
	unsigned short LDIR_Name1[5];        /* 文件名第1-5个字符 */
	unsigned char LDIR_Attr;             /* 文件属性(必须为ATTR_LONG_NAME) */
	unsigned char LDIR_Type;             /* 如果为0，则说明是长目录子项 */
	unsigned char LDIR_Chksum;           /* 短文件名校验和 */
	unsigned short LDIR_Name2[6];        /* 文件名第6-11个字符 */
	unsigned short LDIR_FstClusLO;       /* 值为0 */
	unsigned short LDIR_Name3[2];        /* 文件名第12、13个字符 */
}__attribute__((packed));

void DISK1_FAT32_FS_init();

#endif
