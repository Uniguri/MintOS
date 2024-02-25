#include "ISR.h"

#define SAVE_CONTEXT_STRING                           \
  "push rbp;"                                         \
  "mov rbp, rsp;"                                     \
  "push rax;"                                         \
  "push rbx;"                                         \
  "push rcx;"                                         \
  "push rdx;"                                         \
  "push rdi;"                                         \
  "push rsi;"                                         \
  "push r8;"                                          \
  "push r9;"                                          \
  "push r10;"                                         \
  "push r11;"                                         \
  "push r12;"                                         \
  "push r13;"                                         \
  "push r14;"                                         \
  "push r15;"                                         \
  "mov ax, ds;"                                       \
  "push rax;"                                         \
  "mov ax, es;"                                       \
  "push rax;"                                         \
  "push fs;"                                          \
  "push gs;"                                          \
  "mov ax, 0x10;" /* Kernel Data Segment Descriptor*/ \
  "mov ds, ax;"                                       \
  "mov es, ax;"                                       \
  "mov gs, ax;"                                       \
  "mov fs, ax;"
#define SAVE_CONTEXT() asm volatile(SAVE_CONTEXT_STRING ::)

#define LOAD_CONTEXT_STRING \
  "pop gs;"                 \
  "pop fs;"                 \
  "pop rax;"                \
  "mov es, ax;"             \
  "pop rax;"                \
  "mov ds, ax;"             \
  "pop r15;"                \
  "pop r14;"                \
  "pop r13;"                \
  "pop r12;"                \
  "pop r11;"                \
  "pop r10;"                \
  "pop r9;"                 \
  "pop r8;"                 \
  "pop rsi;"                \
  "pop rdi;"                \
  "pop rdx;"                \
  "pop rcx;"                \
  "pop rbx;"                \
  "pop rax;"                \
  "pop rbp;"
#define LOAD_CONTEXT() asm volatile(LOAD_CONTEXT_STRING ::)

#undef SAVE_CONTEXT_STRING
#undef SAVE_CONTEXT
#undef LOAD_CONTEXT_STRING
#undef LOAD_CONTEXT