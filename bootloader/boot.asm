; ---------------------
;	boot.asm
;		@Kalinote
; ---------------------

	org 0x7c00
	
BaseOfStack		equ		0x7c00

BaseOfLoader	equ		0x1000
OffsetOfLoader	equ		0x00

RootDirSectors		equ		14
SectorNumOfRootDirStart		equ		19
SectorNumOfFAT1Start		equ		1
SectorBalance		equ		17	

; FAT12格式
	jmp	short Label_Start				; 跳转到启动代码
	nop
	BS_OEMName		db	'KALIBOOT'			; 启动区名称(8字节)
	BPB_BytesPerSec	dw	512					; 每个扇区大小为512
	BPB_SecPerClus	db	1					; 每个簇大小为一个扇区
	BPB_RsvdSecCnt	dw	1					; FAT起始位置(boot记录占用扇区数)
	BPB_NumFATs		db	2                   ; FAT表数
	BPB_RootEntCnt	dw	224					; 根目录最大文件数
	BPB_TotSec16	dw	2880				; 磁盘大小(总扇区数，如果这里扇区数为0，则由下面给出)
	BPB_Media		db	0xf0                ; 磁盘种类
	BPB_FATSz16		dw	9                   ; FAT长度(每个FAT表占用扇区数)
	BPB_SecPerTrk	dw	18					; 每个磁道的扇区数
	BPB_NumHeads	dw	2					; 磁头数(面数)
	BPB_HiddSec		dd	0                   ; 不使用分区(隐藏扇区数)
	BPB_TotSec32	dd	0					; 重写一遍磁盘大小(如果上面的扇区数为0，则由这里给出)
	BS_DrvNum		db	0                   ; INT 13H的驱动器号
	BS_Reserved1	db	0					; 保留
	BS_BootSig		db	0x29                ; 扩展引导标记(29h)磁盘名称(11字节)
	BS_VolID		dd	0                   ; 卷序列号
	BS_VolLab		db	'KALINOTEOS2'       ; 磁盘名称(11字节)
	BS_FileSysType	db	'FAT12   '			; 磁盘格式名称(8字节)

Label_Start:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,BaseOfStack
	
; 调用BIOS功能
; 清屏
	mov ax,0600h
	mov bx,0700h
	mov cx,0
	mov dx,0184fh
	int 10h
	
; 设置光标位置
	mov ax,0200h
	mov bx,0000h
	mov cx,0000h
	int 10h
	
; 显示start booting
	mov ax,1301h
	mov bx,000fh
	mov dx,0000h
	mov cx,10
	push ax
	mov ax,ds
	mov es,ax
	pop ax
	mov bp,StratBootMessage
	int 10h
	
; 软盘复位
	xor ah,ah
	xor dl,dl
	int 13h

; 搜索 loader.bin
	mov	word	[SectorNo],	SectorNumOfRootDirStart

Lable_Search_In_Root_Dir_Begin:

	cmp	word	[RootDirSizeForLoop],	0
	jz	Label_No_LoaderBin
	dec	word	[RootDirSizeForLoop]	
	mov	ax,	00h
	mov	es,	ax
	mov	bx,	8000h
	mov	ax,	[SectorNo]
	mov	cl,	1
	call	Func_ReadOneSector
	mov	si,	LoaderFileName
	mov	di,	8000h
	cld
	mov	dx,	10h
	
Label_Search_For_LoaderBin:

	cmp	dx,	0
	jz	Label_Goto_Next_Sector_In_Root_Dir
	dec	dx
	mov	cx,	11

Label_Cmp_FileName:

	cmp	cx,	0
	jz	Label_FileName_Found
	dec	cx
	lodsb	
	cmp	al,	byte	[es:di]
	jz	Label_Go_On
	jmp	Label_Different

Label_Go_On:
	
	inc	di
	jmp	Label_Cmp_FileName

Label_Different:

	and	di,	0ffe0h
	add	di,	20h
	mov	si,	LoaderFileName
	jmp	Label_Search_For_LoaderBin

Label_Goto_Next_Sector_In_Root_Dir:
	
	add	word	[SectorNo],	1
	jmp	Lable_Search_In_Root_Dir_Begin
	
; 在屏幕上显示 ERROR:No LOADER Found

Label_No_LoaderBin:

	mov	ax,	1301h
	mov	bx,	008ch
	mov	dx,	0100h
	mov	cx,	21
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	NoLoaderMessage
	int	10h
	jmp	$

; 在根目录结构中找到loader.bin

Label_FileName_Found:

	mov	ax,	RootDirSectors
	and	di,	0ffe0h
	add	di,	01ah
	mov	cx,	word	[es:di]
	push	cx
	add	cx,	ax
	add	cx,	SectorBalance
	mov	ax,	BaseOfLoader
	mov	es,	ax
	mov	bx,	OffsetOfLoader
	mov	ax,	cx

Label_Go_On_Loading_File:
	push	ax
	push	bx
	mov	ah,	0eh
	mov	al,	'.'
	mov	bl,	0fh
	int	10h
	pop	bx
	pop	ax

	mov	cl,	1
	call	Func_ReadOneSector
	pop	ax
	call	Func_GetFATEntry
	cmp	ax,	0fffh
	jz	Label_File_Loaded
	push	ax
	mov	dx,	RootDirSectors
	add	ax,	dx
	add	ax,	SectorBalance
	add	bx,	[BPB_BytesPerSec]
	jmp	Label_Go_On_Loading_File

Label_File_Loaded:
	
	jmp	BaseOfLoader:OffsetOfLoader

; 从软盘读取一个扇区

Func_ReadOneSector:
	
	push	bp
	mov	bp,	sp
	sub	esp,	2
	mov	byte	[bp - 2],	cl
	push	bx
	mov	bl,	[BPB_SecPerTrk]
	div	bl
	inc	ah
	mov	cl,	ah
	mov	dh,	al
	shr	al,	1
	mov	ch,	al
	and	dh,	1
	pop	bx
	mov	dl,	[BS_DrvNum]
Label_Go_On_Reading:
	mov	ah,	2
	mov	al,	byte	[bp - 2]
	int	13h
	jc	Label_Go_On_Reading
	add	esp,	2
	pop	bp
	ret

; 获得FAT表

Func_GetFATEntry:

	push	es
	push	bx
	push	ax
	mov	ax,	00
	mov	es,	ax
	pop	ax
	mov	byte	[Odd],	0
	mov	bx,	3
	mul	bx
	mov	bx,	2
	div	bx
	cmp	dx,	0
	jz	Label_Even
	mov	byte	[Odd],	1

Label_Even:

	xor	dx,	dx
	mov	bx,	[BPB_BytesPerSec]
	div	bx
	push	dx
	mov	bx,	8000h
	add	ax,	SectorNumOfFAT1Start
	mov	cl,	2
	call	Func_ReadOneSector
	
	pop	dx
	add	bx,	dx
	mov	ax,	[es:bx]
	cmp	byte	[Odd],	1
	jnz	Label_Even_2
	shr	ax,	4

Label_Even_2:
	and	ax,	0fffh
	pop	bx
	pop	es
	ret

; 临时变量
RootDirSizeForLoop	dw	RootDirSectors
SectorNo			dw	0
Odd					db	0

; 显示信息
StratBootMessage:
	db "Start Boot"
NoLoaderMessage:
	db "ERROR:No LOADER Found"
LoaderFileName:
	db "loader  bin",0

; 填充0，直到510字节(最后两个字节是固定标识符)
	times 510-($-$$) db 0
	dw 0xaa55
	