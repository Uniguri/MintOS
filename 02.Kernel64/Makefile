# Makefile
NASM = nasm -f elf64
GCC = gcc -c -m64 -ffreestanding -masm=intel -g
LD = ld -melf_x86_64 -T ../elf_x86_64.x -nostdlib -e Main -Ttext 0x200000

OBJCOPY = objcopy -j .text -j .data -j .rodata -j .bss -S -O binary

OBJ_DIR = Temp
SRC_DIR	= Source

all: prepare Kernel64.bin

prepare:
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/EntryPoint.bin: $(SRC_DIR)/EntryPoint.s
	$(NASM) -o $@ $<

dep:
	make -C $(OBJ_DIR) -f ../Makefile InternalDependency

ExecuteInternalBuild: dep
	make -C $(OBJ_DIR) -f ../Makefile Kernel64.elf

Kernel64.bin: ExecuteInternalBuild
	$(OBJCOPY) $(OBJ_DIR)/Kernel64.elf $@
		
clean:
	rm -f *.bin
	rm -rf $(OBJ_DIR)


# Workspace is Temp while below code is executed.
ENTRYPOINT_SRC = ../$(SRC_DIR)/EntryPoint.s
ENTRYPOINT_OBJ = EntryPoint.o
C_SRCS = $(wildcard ../$(SRC_DIR)/*.c)
ASM_SRCS = $(wildcard ../$(SRC_DIR)/*.asm)
C_OBJS = $(notdir $(patsubst %.c,%.o,$(C_SRCS)))
ASM_OBJS = $(notdir $(patsubst %.asm,%.o,$(ASM_SRCS)))

$(ENTRYPOINT_OBJ): $(ENTRYPOINT_SRC)
	$(NASM) -o $@ $<

%.o: ../$(SRC_DIR)/%.c
	$(GCC) -c $<

%.o: ../$(SRC_DIR)/%.asm
	$(NASM) -o $@ $<

InternalDependency:
	$(GCC) -MM $(C_SRCS) > Dependency.dep

Kernel64.elf: $(ENTRYPOINT_OBJ) $(C_OBJS) $(ASM_OBJS)
	$(LD) -o $@ $^

ifeq (Dependency.dep, $(wildcard Dependency.dep))
include Dependency.dep
endif