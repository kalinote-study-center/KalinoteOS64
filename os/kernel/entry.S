#include <linkage.h>


/*
* 这个文件的程序用于处理中断、异常、陷阱、系统调用、特权级切换、进程调度等程序间的调用和切换
*/

/* 各寄存器在栈中保存的位置 */
R15	=	0x00
R14	=	0x08
R13	=	0x10
R12	=	0x18
R11	=	0x20
R10	=	0x28
R9	=	0x30
R8	=	0x38
RBX	=	0x40
RCX	=	0x48
RDX	=	0x50
RSI	=	0x58
RDI	=	0x60
RBP	=	0x68
DS	=	0x70
ES	=	0x78
RAX	=	0x80
FUNC	=	0x88
ERRCODE	=	0x90
RIP	=	0x98
CS	=	0xa0
RFLAGS	=	0xa8
OLDRSP	=	0xb0
OLDSS	=	0xb8

/* PCB结构体中相关数据的偏移 */
TSK_STATE	=	0x00
TSK_FLAGS	=	0x08
TSK_PREEMPT	=	0x10
TSK_SIGNAL	=	0x18

RESTORE_ALL:
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rbx
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%rax	
	movq	%rax,	%ds
	popq	%rax
	movq	%rax,	%es
	popq	%rax
	addq	$0x10,	%rsp
	iretq

/**
* #define GET_CURRENT(reg)	\
* 	movq	$-32768,reg;	\
* 	andq	%rsp,	reg
*/
/* now成员在task_schedule中的偏移量为0x10 */
#define GET_CURRENT(reg)		\
	movq	now_task(%rip),reg

ret_from_exception:
	
ENTRY(ret_from_intr)
	movq	$-1,	%rcx
	testq	softirq_status(%rip),	%rcx	////check softirq
	jnz	softirq_handler
	GET_CURRENT(%rbx)
	movq	TSK_PREEMPT(%rbx),	%rcx		////check preempt
	cmpq	$0,	%rcx
	jne		RESTORE_ALL
	movq	TSK_FLAGS(%rbx),	%rcx		////try schedule
	testq	$2,	%rcx
	jnz	reschedule
	jmp	RESTORE_ALL	

softirq_handler:
	callq	do_softirq
	GET_CURRENT(%rbx)
	movq	TSK_PREEMPT(%rbx),	%rcx		////check preempt
	cmpq	$0,	%rcx
	jne		RESTORE_ALL
	movq	TSK_FLAGS(%rbx),	%rcx		////try schedule
	testq	$2,	%rcx
	jnz	reschedule
	jmp	RESTORE_ALL		

reschedule:
	callq	schedule						////do schedule
	jmp	RESTORE_ALL


/* 系统调用 */
ENTRY(system_call)
	sti								/* 调用SYSENTER指令会复位IF中断标志位，导致无法接受中断，在这里进行开启 */
	subq	$0x38,	%rsp
	cld

	pushq	%rax
	movq	%es,	%rax
	pushq	%rax
	movq	%ds,	%rax
	pushq	%rax
	xorq	%rax,	%rax
	pushq	%rbp
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15

	movq	$0x10,	%rax
	movq	%rax,	%ds
	movq	%rax,	%es

	movq	RAX(%rsp),	%rax
	leaq	system_call_table(%rip),	%rbx
	callq	*(%rbx,%rax,8)
	movq	%rax,	RAX(%rsp)

ENTRY(ret_system_call) 
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rbx
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%rax
	movq	%rax,	%ds
	popq	%rax
	movq	%rax,	%es
	popq	%rax
	addq	$0x38,	%rsp

	xchgq	%rdx,	%r10
	xchgq	%rcx,	%r11

	sti
	.byte	0x48
	sysexit


/* 下面是错误及异常 */
ENTRY(divide_error)
	pushq	$0
	pushq	%rax
	leaq	do_divide_error(%rip),	%rax
	xchgq	%rax,	(%rsp)

error_code:
	pushq	%rax
	movq	%es,	%rax
	pushq	%rax
	movq	%ds,	%rax
	pushq	%rax
	xorq	%rax,	%rax

	pushq	%rbp
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15	
	
	cld
	movq	ERRCODE(%rsp),	%rsi
	movq	FUNC(%rsp),	%rdx	

	movq	$0x10,	%rdi
	movq	%rdi,	%ds
	movq	%rdi,	%es

	movq	%rsp,	%rdi
	////GET_CURRENT(%ebx)

	callq 	*%rdx

	jmp	ret_from_exception	

ENTRY(debug)
	pushq	$0
	pushq	%rax
	leaq	do_debug(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(nmi)
	pushq	%rax
	cld;
	pushq	%rax

	pushq	%rax
	movq	%es,	%rax
	pushq	%rax
	movq	%ds,	%rax
	pushq	%rax
	xorq	%rax,	%rax

	pushq	%rbp
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15

	movq	$0x10,	%rdx
	movq	%rdx,	%ds
	movq	%rdx,	%es

	movq	$0,	%rsi
	movq	%rsp,	%rdi

	callq	do_nmi

	jmp	RESTORE_ALL

ENTRY(int3)
	pushq	$0
	pushq	%rax
	leaq	do_int3(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(overflow)
	pushq	$0
	pushq	%rax
	leaq	do_overflow(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(bounds)
	pushq	$0
	pushq	%rax
	leaq	do_bounds(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(undefined_opcode)
	pushq	$0
	pushq	%rax
	leaq	do_undefined_opcode(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(dev_not_available)	/* need rewrite */
	pushq	$0
	pushq	%rax
	leaq	do_dev_not_available(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code


ENTRY(double_fault)
	pushq	%rax
	leaq	do_double_fault(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(coprocessor_segment_overrun)
	pushq	$0
	pushq	%rax
	leaq	do_coprocessor_segment_overrun(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(invalid_TSS)
	pushq	%rax
	leaq	do_invalid_TSS(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(segment_not_present)
	pushq	%rax
	leaq	do_segment_not_present(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(stack_segment_fault)
	pushq	%rax
	leaq	do_stack_segment_fault(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(general_protection)
	pushq	%rax
	leaq	do_general_protection(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(page_fault)
	pushq	%rax
	leaq	do_page_fault(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(x87_FPU_error)
	pushq	$0
	pushq	%rax
	leaq	do_x87_FPU_error(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(alignment_check)
	pushq	%rax
	leaq	do_alignment_check(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(machine_check)
	pushq	$0
	pushq	%rax
	leaq	do_machine_check(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(SIMD_exception)
	pushq	$0
	pushq	%rax
	leaq	do_SIMD_exception(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(virtualization_exception)
	pushq	$0
	pushq	%rax
	leaq	do_virtualization_exception(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code


