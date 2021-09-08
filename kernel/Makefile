CFLAGS = -fno-stack-protector -mcmodel=large -fno-builtin -m64

OBJ= head.o main.o printk.o trap.o entry.o memory.o interrupt.o task.o cpu.o 8259A.o APIC.o \
	APU_boot.o disk.o keyboard.o mouse.o SMP.o

all: system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

system:	$(OBJ)
	ld -b elf64-x86-64 -z muldefs -o system $(OBJ) -T Kernel.lds 

head.o:	head.S
	gcc -E  $< > head.ss
	as --64 -o $@ head.ss
	
entry.o: entry.S
	gcc -E  $< > entry.ss
	as --64 -o $@ entry.ss
	
APU_boot.o: APU_boot.S
	gcc -E  $< > APU_boot.ss
	as --64 -o $@ APU_boot.ss

main.o:	main.c
	gcc  $(CFLAGS) -c $<
	
printk.o: printk.c
	gcc  $(CFLAGS) -c $<

trap.o: trap.c
	gcc  $(CFLAGS) -c $<
	
memory.o: memory.c
	gcc  $(CFLAGS) -c $<

interrupt.o: interrupt.c
	gcc  $(CFLAGS) -c $<

task.o: task.c
	gcc  $(CFLAGS) -c $<

cpu.o: cpu.c
	gcc  $(CFLAGS) -c $<

8259A.o: 8259A.c
	gcc  $(CFLAGS) -c $<

APIC.o: APIC.c
	gcc  $(CFLAGS) -c $<

keyboard.o: keyboard.c
	gcc  $(CFLAGS) -c $<

mouse.o: mouse.c
	gcc  $(CFLAGS) -c $<

SMP.o: SMP.c
	gcc  $(CFLAGS) -c $<

clean:
	rm -rf *.o *.ss~ *.ss *.S~ *.c~ *.h~ system  Makefile~ Kernel.lds~ kernel.bin 