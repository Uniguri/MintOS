#include "Task.h"

#include "Descriptor.h"
#include "Macro.h"
#include "Memory.h"
#include "Types.h"

void kSetUpTask(TaskControlBlock* tcb, uint64 id, uint64 flags,
                uint64 entry_point_addr, void* stack_addr, uint64 stack_size) {
  memset(tcb->reg_context, 0, sizeof(tcb->reg_context));
  tcb->context.rsp = (uint64)stack_addr + stack_size;
  tcb->context.rbp = (uint64)stack_addr + stack_size;

  tcb->context.cs = GDT_KERNEL_CODE_SEGMENT;
  tcb->context.ds = GDT_KERNEL_DATA_SEGMENT;
  tcb->context.es = GDT_KERNEL_DATA_SEGMENT;
  tcb->context.fs = GDT_KERNEL_DATA_SEGMENT;
  tcb->context.gs = GDT_KERNEL_DATA_SEGMENT;
  tcb->context.ss = GDT_KERNEL_DATA_SEGMENT;

  tcb->context.rip = entry_point_addr;

  // Set IF(9)bit.
  tcb->context.rflags |= BIT(9);

  tcb->id = id;
  tcb->stack_addr = stack_addr;
  tcb->stack_size = stack_size;
  tcb->flags = flags;
}
#define SAVE_CONTEXT_STRING \
  "push rbp;"               \
  "push rax;"               \
  "push rbx;"               \
  "push rcx;"               \
  "push rdx;"               \
  "push rdi;"               \
  "push rsi;"               \
  "push r8;"                \
  "push r9;"                \
  "push r10;"               \
  "push r11;"               \
  "push r12;"               \
  "push r13;"               \
  "push r14;"               \
  "push r15;"               \
  "mov ax, ds;"             \
  "push rax;"               \
  "mov ax, es;"             \
  "push rax;"               \
  "push fs;"                \
  "push gs;"
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