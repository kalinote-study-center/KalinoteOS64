	org 0x7c00
	
BaseOfStack		equ		0x7c00

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
	
	jmp $
	
StratBootMessage:
	db "Start Boot"

; 填充0，直到510字节(最后两个字节是固定标识符)
	times 510-($-$$) db 0
	dw 0xaa55
	