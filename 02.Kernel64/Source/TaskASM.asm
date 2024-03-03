[BITS 64]

SECTION .text

global kSwitchContext

%macro SAVE_CONTEXT 0 
    push rbp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    mov ax, ds
    push rax
    mov ax, es
    push rax
    push fs
    push gs 
%endmacro

%macro LOAD_CONTEXT 0
    pop gs
    pop fs
    pop rax
    mov es, ax
    pop rax
    mov ds, ax
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp        
%endmacro

; void kSwitchContext(Context* current_context, Context* next_context)
kSwitchContext:
  ; Prologue
  push rbp
  mov rbp, rsp

  ; Check current_context == nullptr
  pushfq
  cmp rdi, 0
  je .LoadContext ; Jump .LoadContext if current_context == nullptr.
  popfq

  push rax ; backup rax
  ; Save SS, RSP, RFLAGS, CS, RIP in order
  mov ax, ss 
  mov qword [rdi + (23 * 8)], rax ; Save SS

  mov rax, rbp
  add rax, 0x10 ; Ignore RBP in prologue and RET.
  mov qword [rdi + (22 * 8)], rax ; Save RSP

  pushfq
  pop rax
  mov qword [rdi + (21 * 8)], rax ; Save RFLAGS

  mov ax, cs
  mov qword [rdi + (20 * 8)], rax ; Save CS

  mov rax, qword[ rbp + 8 ] ; RBP + 8 -> RET
  mov qword [rdi + (19 * 8)], rax  ; Save RIP (RIP = RET of this(kSwitchContext) function).

  ; Restore backuped regs.
  pop rax
  pop rbp

  ; Move RSP to save regs by pushing, since we already save SS, RSP, RFLAGS, CS, RIP.
  add rdi, (19 * 8)
  mov rsp, rdi
  sub rdi, (19 * 8)

  ; Save other regs.
  SAVE_CONTEXT

  .LoadContext:
    mov rsp, rsi

    LOAD_CONTEXT
    iretq