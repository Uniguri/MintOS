#include "Task.h"

#include "Descriptor.h"
#include "Interrupt.h"
#include "Macro.h"
#include "Memory.h"
#include "Types.h"

static Scheduler scheduler;
static TCBPoolManager tcb_pool_manager;

void kInitializeTCBPool(void) {
  memset(&tcb_pool_manager, 0, sizeof(TCBPoolManager));

  tcb_pool_manager.start_addr = (TaskControlBlock*)TASK_TCB_POOL_ADDRESS;
  memset((void*)TASK_TCB_POOL_ADDRESS, 0,
         sizeof(TaskControlBlock) * TASK_MAX_COUNT);

  for (uint64 i = 0; i < TASK_MAX_COUNT; ++i) {
    tcb_pool_manager.start_addr[i].link.id = i;
  }

  tcb_pool_manager.max_count = TASK_MAX_COUNT;
  tcb_pool_manager.alloacted_count = 0;
}

TaskControlBlock* kAllocateTCB(void) {
  if (tcb_pool_manager.use_count == tcb_pool_manager.max_count) {
    return nullptr;
  }

  TaskControlBlock* tcb;
  uint64 id;
  for (uint64 i = 0; i < tcb_pool_manager.max_count; ++i) {
    if (!IS_BIT_SET(tcb_pool_manager.start_addr[i].link.id, 32)) {
      tcb = &tcb_pool_manager.start_addr[i];
      id = tcb->link.id;
      break;
    }
  }

  tcb->link.id = (BIT(32llu) | id);
  ++tcb_pool_manager.use_count;
  ++tcb_pool_manager.alloacted_count;
  if (tcb_pool_manager.alloacted_count == 0) {
    tcb_pool_manager.alloacted_count = 1;
  }

  return tcb;
}

void kFreeTCB(uint64 id) {
  uint64 i = id & 0xFFFFFFFF;
  memset(&tcb_pool_manager.start_addr[i], 0, sizeof(Context));
  tcb_pool_manager.start_addr[i].link.id = i;
  --tcb_pool_manager.use_count;
}

TaskControlBlock* kCreateTask(uint64 flags, uint64 entry_point_addr) {
  TaskControlBlock* task = kAllocateTCB();
  if (!task) {
    return nullptr;
  }

  void* stack_addr = (void*)(TASK_STACK_POOL_ADDRESS +
                             TASK_STACK_SIZE * (task->link.id & 0xFFFFFFFF));
  kSetUpTask(task, flags, entry_point_addr, stack_addr, TASK_STACK_SIZE);
  kAddTaskToReadyList(task);
  return task;
}

void kSetUpTask(TaskControlBlock* tcb, uint64 flags, uint64 entry_point_addr,
                void* stack_addr, uint64 stack_size) {
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

  tcb->stack_addr = stack_addr;
  tcb->stack_size = stack_size;
  tcb->flags = flags;
}

inline void kInitializeScheduler(void) {
  kInitializeTCBPool();
  kInitializeList(&scheduler.ready_list);
  scheduler.running_task = kAllocateTCB();
}

inline void kSetRunningTask(TaskControlBlock* task) {
  scheduler.running_task = task;
}

inline TaskControlBlock* kGetRunningTask(void) {
  return scheduler.running_task;
}

inline TaskControlBlock* kGetNextTaskToRun(void) {
  if (!kGetListCount(&scheduler.ready_list)) {
    return nullptr;
  }
  return (TaskControlBlock*)kRemoveListFromHeader(&scheduler.ready_list);
}

inline void kAddTaskToReadyList(TaskControlBlock* task) {
  kAddListToTail(&scheduler.ready_list, task);
}

void kSchedule(void) {
  if (!kGetListCount(&scheduler.ready_list)) {
    return;
  }

  bool prev_flag = kIsInterruptEnabled();
  kSetInterruptFlag(false);

  TaskControlBlock* next_task = kGetNextTaskToRun();
  if (!next_task) {
    kSetInterruptFlag(prev_flag);
    return;
  }

  TaskControlBlock* running_tast = scheduler.running_task;
  kAddTaskToReadyList(running_tast);

  scheduler.running_task = next_task;
  kSwitchContext(&running_tast->context, &next_task->context);

  scheduler.processor_time = TASK_PROCESSOR_TIME;
  kSetInterruptFlag(prev_flag);
}

bool kScheduleInInterrupt(void) {
  TaskControlBlock* next_task = kGetNextTaskToRun();
  if (!next_task) {
    return false;
  }
  // printf("next_task = %p, %d\n", next_task,
  //        kGetListCount(&scheduler.ready_list));

  char* context_addr = (char*)IST_START_ADDRESS + IST_SIZE - sizeof(Context);

  TaskControlBlock* running_task = scheduler.running_task;
  memcpy(&running_task->context, context_addr, sizeof(Context));
  kAddTaskToReadyList(running_task);

  scheduler.running_task = next_task;
  memcpy(context_addr, &next_task->context, sizeof(Context));

  scheduler.processor_time = TASK_PROCESSOR_TIME;
  return true;
}

inline void kDecreaseProcessorTime(void) {
  if (scheduler.processor_time > 0) {
    --scheduler.processor_time;
  }
}

inline bool kIsProcessorTimeExpired(void) {
  return scheduler.processor_time <= 0;
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