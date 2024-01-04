all: VirtualOS.bin

VirtualOS.bin: VirtualOS.asm
	nasm -f bin -o VirtualOS.bin VirtualOS.asm

clean:
	rm -f VirtualOS.bin