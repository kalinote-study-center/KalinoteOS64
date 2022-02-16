FAT12IMG=tools/fat12img
OS= ./os/
BIN= $(OS)bin/

# 需要拷到硬盘的文件
DISK_FILES= $(BIN)LOADER.BIN \
			$(BIN)KERNEL.BIN

all: os

.PHONY: os

kalinote.vfd: os
	$(FAT12IMG) $@ format
	$(FAT12IMG) $@ write $(BIN)boot.bin 0
	for filename in $(DISK_FILES); do \
	  $(FAT12IMG) $@ save $$filename; \
	done

os:
	make -C os
	
run: kalinote.vfd
	tools/bochsdbg.exe
	
clean:
	make -C os			clean
	rm -rf *.vfd
