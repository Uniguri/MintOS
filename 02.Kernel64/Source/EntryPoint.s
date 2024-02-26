[BITS 64]

SECTION .text

; Main of Kernel64. Implemented on ./Main.c
extern Main

START:
  ; Set Segment Selector to IA32e_DATA_DESCRIPTOR defined in 01.Kernel32/Source/EntryPoint.s.
  mov ax, 0x10 ; IA32e_DATA_DESCRIPTOR
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  
  mov ss, ax
  mov rsp, 0x6FFFF8
  mov rbp, 0x6FFFF8

  ; Call Main of Kernel64. Implemented on ./Main.c
  call Main
  
  jmp $