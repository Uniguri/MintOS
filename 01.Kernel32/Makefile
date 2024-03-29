# Makefile
NASM = nasm -f bin
GCC = gcc -c -m32 -ffreestanding -masm=intel
LD = ld -melf_i386 -T ../elf_i386.x -nostdlib -e Main -Ttext 0x10200

OBJCOPY = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJ_DIR = Temp
SRC_DIR	= Source

all: prepare Kernel32.bin

prepare:
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/EntryPoint.bin: $(SRC_DIR)/EntryPoint.s
	$(NASM) -o $@ $<

dep:
	make -C $(OBJ_DIR) -f ../Makefile InternalDependency

ExecuteInternalBuild: dep
	make -C $(OBJ_DIR) -f ../Makefile Kernel32.elf

$(OBJ_DIR)/Kernel32.elf.bin: ExecuteInternalBuild
	$(OBJCOPY) $(OBJ_DIR)/Kernel32.elf $@

Kernel32.bin: $(OBJ_DIR)/EntryPoint.bin $(OBJ_DIR)/Kernel32.elf.bin
	cat $^ > $@
		
clean:
	rm -f *.bin
	rm -rf $(OBJ_DIR)


C_ENTRYPOINT_OBJ = Main.o
# Workspace is Temp while below code is executed
C_SRCS = $(wildcard ../$(SRC_DIR)/*.c)
ASM_SRCS = $(wildcard ../$(SRC_DIR)/*.asm)
C_OBJS = $(subst Main.o, , $(notdir $(patsubst %.c,%.o,$(C_SRCS))))
ASM_OBJS = $(notdir $(patsubst %.asm,%.o,$(ASM_SRCS)))

%.o: ../$(SRC_DIR)/%.c
	$(GCC) -c $<

%.o: ../$(SRC_DIR)/%.asm
	$(NASM) -f elf32 -o $@ $<

InternalDependency:
	$(GCC) -MM $(C_SRCS) > Dependency.dep

Kernel32.elf: $(C_ENTRYPOINT_OBJ) $(C_OBJS) $(ASM_OBJS)
	$(LD) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif