FAT12IMG=tools/fat12img
BOOTLOADER= ./bootloader/
KERNEL= ./kernel/

# 需要拷到硬盘的文件
DISK_FILES= $(BOOTLOADER)loader.bin \
			$(KERNEL)kernel.bin

all: kalinote.vfd

.PHONY: bootloader kernel

kalinote.vfd: bootloader kernel
	$(FAT12IMG) $@ format
	$(FAT12IMG) $@ write $(BOOTLOADER)boot.bin 0
	for filename in $(DISK_FILES); do \
	  $(FAT12IMG) $@ save $$filename; \
	done
	
bootloader:
	make -C bootloader
kernel:
	make -C kernel
	
run: all
	tools/bochsdbg.exe
	
clean:
	make -C bootloader		clean
	make -C kernel			clean
	rm -rf *.vfd