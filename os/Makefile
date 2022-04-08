# Makefile简单说明：
# CFLAGS变量是在系统C语言代码编译时使用的参数
# INCLUDE变量是include文件的路径
# OBJ是所有需要编译的项目
# DRIVER_OBJ是驱动程序相关代码
# GRAPHIC_OBJ是图形界面相关代码
# gcc对*.S文件进行处理时使用-E参数，表示仅进行预处理，在进行预处理时可以添加-C参数表示不删除注释
# 在CFLAGS中，-fno-stack-protector参数表示不检查stack，在某些版本的编译器中，不添加此参数可能会报错
# 在CFLAGS中，-mcmodel=large表示程序可以访问任何虚拟空间
# 在CFLAGS中，-fno-builtin表示不使用任何内建函数，除非添加__builtin_引用
# 在CFLAGS中，-m64表示生成64位代码
# 在CFLAGS中，-c表示进行预处理、编译等，但不进行链接
# 在INCLUDE中，-I表示引用文件位置(include文件夹)
# 在Makefile中，$@表示目标文件，$<表示需求文件
# 在ld中，-b表示输入文件格式，-z表示允许重复定义，-o表示输出文件名，-T表示链接过程提供的脚本文件
# 关于更多ld的信息，可以在以下网站找到：https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_mono/ld.html
# 需要注意的是，在链接的过程中，head.S需要放在第一个(作为代码入口)
# 通过设置PIC := APIC来开启APIC，设置PIC_8259A使用8259A芯片，但是在开发测试环境下硬件设备不支持2xAPIC，开启会触发通用保护异常
# 通过设置PLATFORM来选择目标编译环境[可选项：BOCHS, HYPER_V](因为Hyper-V、bochs、vmware和物理平台有些地方不同)

CFLAGS = -fno-stack-protector -mcmodel=large -fno-builtin -m64 $(INCLUDE)
INCLUDE = -I include
OBJ = obj/head.o obj/entry.o obj/APU_boot.o obj/main.o obj/printk.o obj/trap.o obj/memory.o obj/interrupt.o \
		obj/mtask.o obj/cpu.o obj/PIC.o obj/SMP.o obj/time.o obj/softirq.o obj/timer.o obj/schedule.o \
		obj/fat32.o obj/VFS.o obj/sys.o obj/syscalls.o $(DRIVER_OBJ) $(GRAPHIC_OBJ)

DRIVER_OBJ = obj/keyboard.o obj/mouse.o obj/disk.o obj/PIT.o

GRAPHIC_OBJ = obj/graphic.o obj/layer.o obj/window.o

PIC := APIC
PLATFORM := HYPER_V

all: bin/boot.bin bin/loader.bin system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system bin/kernel.bin

# bootloader
bin/boot.bin : bootloader/boot.asm
	nasm $< -o $@ $(INCLUDE)
ifeq ($(PLATFORM),HYPER_V)
bin/loader.bin : bootloader/loader_hyper_v.asm
	nasm $< -o $@ $(INCLUDE)
else
bin/loader.bin : bootloader/loader_bochs.asm
	nasm $< -o $@ $(INCLUDE)
endif

# kernel
ifeq ($(PLATFORM),HYPER_V)
obj/head.o:	kernel/head_hyper_v.S
	gcc -E  kernel/head_hyper_v.S > kernel/head_hyper_v.ss $(INCLUDE)
	as --64 -o $@ kernel/head_hyper_v.ss
else
obj/head.o:	kernel/head_bochs.S
	gcc -E  kernel/head_bochs.S > kernel/head_bochs.ss $(INCLUDE)
	as --64 -o $@ kernel/head_bochs.ss
endif
obj/entry.o:	kernel/entry.S
	gcc -E  kernel/entry.S > kernel/entry.ss $(INCLUDE)
	as --64 -o $@ kernel/entry.ss
obj/APU_boot.o:	kernel/APU_boot.S
	gcc -E  kernel/APU_boot.S > kernel/APU_boot.ss $(INCLUDE)
	as --64 -o $@ kernel/APU_boot.ss
obj/main.o:	kernel/main.c
	gcc  $(CFLAGS) -c $< -o $@ -D$(PIC) -D$(PLATFORM)
obj/printk.o:	kernel/printk.c
	gcc  $(CFLAGS) -c $< -o $@
obj/trap.o:	kernel/trap.c
	gcc  $(CFLAGS) -c $< -o $@
obj/memory.o:	kernel/memory.c
	gcc  $(CFLAGS) -c $< -o $@
obj/interrupt.o:	kernel/interrupt.c
	gcc  $(CFLAGS) -c $< -o $@
obj/mtask.o:	kernel/mtask.c
	gcc  $(CFLAGS) -c $< -o $@
obj/cpu.o:	kernel/cpu.c
	gcc  $(CFLAGS) -c $< -o $@
ifeq ($(PIC),APIC)
obj/PIC.o:	kernel/APIC.c
	gcc  $(CFLAGS) -c $< -o $@
else
obj/PIC.o:	kernel/8259A.c
	gcc  $(CFLAGS) -c $< -o $@
endif
obj/SMP.o:	kernel/SMP.c
	gcc  $(CFLAGS) -c $< -o $@
obj/time.o:	kernel/time.c
	gcc  $(CFLAGS) -c $< -o $@
obj/softirq.o:	kernel/softirq.c
	gcc  $(CFLAGS) -c $< -o $@
obj/timer.o:	kernel/timer.c
	gcc  $(CFLAGS) -c $< -o $@
obj/schedule.o:	kernel/schedule.c
	gcc  $(CFLAGS) -c $< -o $@
obj/fat32.o:	kernel/fat32.c
	gcc  $(CFLAGS) -c $< -o $@
obj/VFS.o:	kernel/VFS.c
	gcc  $(CFLAGS) -c $< -o $@
obj/sys.o:	kernel/sys.c
	gcc  $(CFLAGS) -c $< -o $@
obj/syscalls.o:	kernel/syscalls.c
	gcc  $(CFLAGS) -c $< -o $@

# driver
obj/keyboard.o:	driver/keyboard.c
	gcc  $(CFLAGS) -c $< -o $@
obj/mouse.o:	driver/mouse.c
	gcc  $(CFLAGS) -c $< -o $@
obj/disk.o:	driver/disk.c
	gcc  $(CFLAGS) -c $< -o $@
obj/PIT.o:	driver/PIT.c
	gcc  $(CFLAGS) -c $< -o $@

# graphic
obj/graphic.o:	graphic/graphic.c
	gcc  $(CFLAGS) -c $< -o $@
obj/layer.o:	graphic/layer.c
	gcc  $(CFLAGS) -c $< -o $@
obj/window.o:	graphic/window.c
	gcc  $(CFLAGS) -c $< -o $@

# system
system:	$(OBJ)
	ld -b elf64-x86-64 -z muldefs -o system $(OBJ) -T kernel/Kernel.lds 

clean:
	rm -rf bin/*.bin
	rm -rf obj/*.o
	rm -rf kernel/*.ss
	rm -rf system
