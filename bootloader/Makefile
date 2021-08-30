
all : boot.bin loader.bin

# bootloader
boot.bin : boot.asm
	nasm $< -o $@
loader.bin : loader.asm
	nasm $< -o $@
	
clean:
	rm -rf *.bin
	