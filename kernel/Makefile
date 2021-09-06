CFLAGS = -fno-stack-protector -mcmodel=large -fno-builtin -m64

OBJ= head.o main.o printk.o trap.o entry.o memory.o interrupt.o task.o cpu.o

all: system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

system:	$(OBJ)
	ld -b elf64-x86-64 -z muldefs -o system $(OBJ) -T Kernel.lds 

head.o:	head.S
	gcc -E  head.S > head.ss
	as --64 -o head.o head.ss
	
entry.o: entry.S
	gcc -E  entry.S > entry.ss
	as --64 -o entry.o entry.ss
	
main.o:	main.c
	gcc  $(CFLAGS) -c main.c
	
printk.o: printk.c
	gcc  $(CFLAGS) -c printk.c

clean:
	rm -rf *.o *.ss~ *.ss *.S~ *.c~ *.h~ system  Makefile~ Kernel.lds~ kernel.bin 