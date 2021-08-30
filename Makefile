FAT12IMG=tools/fat12img

# 需要拷到硬盘的文件
DISK_FILES= loader.bin

all: kalinote.vfd

# bootloader
boot.bin : boot.asm
	nasm $< -o $@
loader.bin : loader.asm
	nasm $< -o $@



kalinote.vfd: boot.bin loader.bin
	$(FAT12IMG) $@ format
	$(FAT12IMG) $@ write boot.bin 0
	for filename in $(DISK_FILES); do \
	  $(FAT12IMG) $@ save $$filename; \
	done
	
clean:
	rm -rf *.bin
	rm -rf *.vfd