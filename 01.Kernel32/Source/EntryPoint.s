[ORG 0x10000]
[BITS 16]

SECTION .text

START:
  mov ax, 0x1000
  mov ds, ax
  mov es, ax

  cli ; Clear Interrupts
  lgdt [GDTR] ; Load GDT

  ; Set CR0 for enabling protected mode.
  ; Check out https://en.wikipedia.org/wiki/Control_register#CR0.
  mov eax, 0x4000003B ; PG=0, CD=1, NW=0, AM=0, WP=0, NE=1, ET=1, TS=1, EM=0, MP=1, PE=1
  mov cr0, eax

  jmp dword 0x18:(PROTECTED_MODE - $$ + 0x10000)

[BITS 32]
PROTECTED_MODE:
  mov ax, 0x20
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  mov ss, ax
  mov esp, 0xFFFE
  mov ebp, 0xFFFE

  push (SWITCH_SUCCESS_MESSAGE - $$ + 0x10000)
  push 2
  push 0
  call PRINT_MESSAGE
  add esp, 12

  jmp dword 0x18:0x10200

; Functions
; void PRINT_MESSAGE(int x, int y, const char* str)
PRINT_MESSAGE:
  push ebp
  mov ebp, esp
  push esi
  push edi
  push eax
  push ecx
  push edx

  mov eax, dword [ebp + 12]
  mov esi, 160
  mul esi
  mov edi, eax

  mov eax, dword [ebp + 8]
  mov esi, 2
  mul esi
  add edi, eax

  mov esi, dword [ebp + 16]

  .MESSAGE_LOOP:
    mov cl, byte [esi]
    cmp cl, 0
    je .MESSAGE_END

    mov byte [edi + 0xB8000], cl
    add esi, 1
    add edi, 2
    jmp .MESSAGE_LOOP
  
  .MESSAGE_END:
    pop edx
    pop ecx
    pop eax
    pop edi
    pop esi
    pop ebp
    ret

; Data
align 8, db 0

; Below is a code defining GDT and GDTR.
; Check out: https://wiki.osdev.org/Global_Descriptor_Table
dw 0x0000
GDTR: ; Global Descriptor Table Register
  dw GDT_END - GDT - 1
  dd (GDT - $$ + 0x10000)

GDT: ; Global Descriptor Table
  NULL_DESCRIPTOR:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x00
    db 0x00
    db 0x00
  
  ; Code Descriptor for IA-32e
  IA32e_CODE_DESCRIPTOR:
    dw 0xFFFF ; Limit
    dw 0x0000 ; Base
    db 0x00   ; Base
    db 0x9A   ; P=1, DPL=0, Code Segment, R-X
    db 0xAF   ; G=1, D=0, L=0, Limit
    db 0x00   ; Base
  
  ; Data Descriptor for IA-32e
  IA32e_DATA_DESCRIPTOR:
    dw 0xFFFF ; Limit
    dw 0x0000 ; Base
    db 0x00   ; Base
    db 0x92   ; P=1, DPL=0, Data Segment, RW-
    db 0xAF   ; G=1, D=0, L=0, Limit
    db 0x00   ; Base

  ; Code Descriptor for protected mode
  CODE_DESCRIPTOR:
    dw 0xFFFF ; Limit
    dw 0x0000 ; Base
    db 0x00   ; Base
    db 0x9A   ; P=1, DPL=0, Code Segment, R-X
    db 0xCF   ; G=1, D=1, L=0, Limit
    db 0x00   ; Base
  
  ; Data Descriptor for protected mode
  DATA_DESCRIPTOR:
    dw 0xFFFF ; Limit
    dw 0x0000 ; Base
    db 0x00   ; Base
    db 0x92   ; P=1, DPL=0, Data Segment, RW-
    db 0xCF   ; G=1, D=1, L=0, Limit
    db 0x00   ; Base 
GDT_END:

SWITCH_SUCCESS_MESSAGE:
  db 'Switch To Protected Mode Success!', 0

times 512 - ($ - $$) db 0x00