FAT12IMG=tools/fat12img

all: kalinote.vfd

boot.bin : boot.asm
	nasm $< -o $@

kalinote.vfd: boot.bin
	$(FAT12IMG) $@ format
	$(FAT12IMG) $@ write boot.bin 0
	
clean:
	rm -rf *.bin
	rm -rf *.vfd