[BITS 32]

global kSwitchAndExecute64bitKernel

SECTION .text

; void kSwitchAndExecute64bitKernel(void)
; Switch system mode to IA-32e and Execute 64 bits kernel.
; Check out https://wiki.osdev.org/Setting_Up_Long_Mode.
kSwitchAndExecute64bitKernel:
  ; Set PAE bit on CR4
  ; Check out: https://en.wikipedia.org/wiki/Control_register#CR4
  mov eax, cr4
  or eax, 0x20 ; Set PAE bit (5 bit)
  mov cr4, eax

  ; Set PML4 addr on CR3
  ; Check out: https://en.wikipedia.org/wiki/Control_register#CR3
  mov eax, 0x100000; PML4 Table addr = 0x100000
  mov cr3, eax

  ; Set LME on IA32_EFER for enabling IA-32e Mode
  ; Check out: https://en.wikipedia.org/wiki/Control_register#EFER
  mov ecx, 0xC0000080 ; EFER's Model Specific Register number = 0xC0000080
  rdmsr ; Read Model Specific Register
  or eax, 0x100 ; Set LME bit (8 bit)
  wrmsr ; Write Model Specific Register

  ; Set NW=0, CD=0, PG=1 on CR0 for enabling cache and paging
  ; Check out: https://en.wikipedia.org/wiki/Control_register#CR0
  mov eax, cr0
  or eax, 0xE0000000 ; Set NW bit (29 bit) and CD bit (30 bit), PG bit (31 bit)
  xor eax, 0x60000000 ; Set NW bit, CD bit to zero
  mov cr0, eax

  jmp 0x08:0x200000

  ; Unreachable
  jmp $