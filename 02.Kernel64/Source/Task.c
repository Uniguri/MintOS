#include "Task.h"

#include "Descriptor.h"
#include "Macro.h"
#include "Memory.h"
#include "Synchronization.h"
#include "Tick.h"
#include "Types.h"

static Scheduler scheduler;
static TCBPoolManager tcb_pool_manager;

void kInitializeTCBPool(void) {
  memset(&tcb_pool_manager, 0, sizeof(TCBPoolManager));

  tcb_pool_manager.tcb = (TaskControlBlock*)TASK_TCB_POOL_ADDRESS;
  memset((void*)TASK_TCB_POOL_ADDRESS, 0,
         sizeof(TaskControlBlock) * TASK_MAX_COUNT);

  for (uint64 i = 0; i < TASK_MAX_COUNT; ++i) {
    tcb_pool_manager.tcb[i].link.id = i;
  }

  tcb_pool_manager.max_count = TASK_MAX_COUNT;
  tcb_pool_manager.alloacted_count = 1;
}

TaskControlBlock* kAllocateTCB(void) {
  if (tcb_pool_manager.use_count == tcb_pool_manager.max_count) {
    return nullptr;
  }

  TaskControlBlock* tcb;
  uint64 id;
  for (uint64 i = 0; i < tcb_pool_manager.max_count; ++i) {
    if (!IS_BIT_SET(tcb_pool_manager.tcb[i].link.id, 32)) {
      tcb = &tcb_pool_manager.tcb[i];
      id = i;
      break;
    }
  }

  tcb->link.id = (TASK_ID_PRESENT | id);
  ++tcb_pool_manager.use_count;
  ++tcb_pool_manager.alloacted_count;
  if (tcb_pool_manager.alloacted_count == 0) {
    tcb_pool_manager.alloacted_count = 1;
  }

  return tcb;
}

inline void kFreeTCB(uint64 id) {
  uint64 offset = GET_TCB_OFFSET_FROM_ID(id);
  memset(&tcb_pool_manager.tcb[offset].context, 0, sizeof(Context));
  tcb_pool_manager.tcb[offset].link.id = offset;
  --tcb_pool_manager.use_count;
}

TaskControlBlock* kCreateTask(uint64 flags, uint64 entry_point_addr) {
  bool prev_flag = kLockForSystemData();
  TaskControlBlock* task = kAllocateTCB();
  if (!task) {
    kUnlockForSystemData(prev_flag);
    return nullptr;
  }
  kUnlockForSystemData(prev_flag);

  void* stack_addr =
      (void*)(TASK_STACK_POOL_ADDRESS +
              TASK_STACK_SIZE * GET_TCB_OFFSET_FROM_ID(task->link.id));
  kSetUpTask(task, flags, entry_point_addr, stack_addr, TASK_STACK_SIZE);

  prev_flag = kLockForSystemData();
  kAddTaskToReadyList(task);
  kUnlockForSystemData(prev_flag);
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

void kInitializeScheduler(void) {
  kInitializeTCBPool();

  for (int i = 0; i < kTaskNumberOfPriority; ++i) {
    kInitializeList(&scheduler.task_to_run_list[i]);
    scheduler.execute_count[i] = 0;
  }
  kInitializeList(&scheduler.task_to_end_list);

  // Set console shell task.
  scheduler.running_task = kAllocateTCB();
  SET_TASK_PRIORITY(scheduler.running_task, kTaskPriorityHighest);

  scheduler.processor_load = 0;
  scheduler.spend_processor_time_ind_idle_task = 0;
}

inline void kSetRunningTask(TaskControlBlock* task) {
  const bool prev_flag = kLockForSystemData();
  scheduler.running_task = task;
  kUnlockForSystemData(prev_flag);
}

inline TaskControlBlock* kGetRunningTask(void) {
  const bool prev_flag = kLockForSystemData();
  TaskControlBlock* running_task = scheduler.running_task;
  kUnlockForSystemData(prev_flag);
  return running_task;
}

TaskControlBlock* kGetNextTaskToRun(void) {
  TaskControlBlock* target = nullptr;
  for (int i = 0; !target && i < 2; ++i) {
    for (int j = 0; j < kTaskNumberOfPriority; ++j) {
      int task_count = kGetListCount(&scheduler.task_to_run_list[j]);
      if (scheduler.execute_count[j] < task_count) {
        target = (TaskControlBlock*)kRemoveListFromHead(
            &scheduler.task_to_run_list[j]);
        ++scheduler.execute_count[j];
        break;
      } else {
        scheduler.execute_count[j] = 0;
      }
    }
  }

  return target;
}

inline bool kAddTaskToReadyList(TaskControlBlock* task) {
  int task_priority = GET_TASK_PRIORITY(task);
  if (task_priority >= kTaskNumberOfPriority) {
    return false;
  }
  kAddListToTail(&scheduler.task_to_run_list[task_priority], task);
  return true;
}

void kSchedule(void) {
  if (!kGetReadyTaskCount()) {
    return;
  }

  const bool prev_flag = kLockForSystemData();
  TaskControlBlock* next_task = kGetNextTaskToRun();
  if (!next_task) {
    kUnlockForSystemData(prev_flag);
    return;
  }

  // Set to run next_task.
  TaskControlBlock* prev_task = scheduler.running_task;
  TaskControlBlock* task_to_run = scheduler.running_task = next_task;

  // If prev task is idle task, increase idle time.
  if (prev_task->flags & TASK_FLAG_IDLE) {
    scheduler.spend_processor_time_ind_idle_task +=
        TASK_PROCESSOR_TIME - scheduler.processor_time;
  }

  // If prev task is to end, we don't need to save context of prev task (current
  // context).
  if (prev_task->flags & TASK_FLAG_END_TASK) {
    kAddListToTail(&scheduler.task_to_end_list, prev_task);
    kSwitchContext(nullptr, &task_to_run->context);
  } else {
    kAddTaskToReadyList(prev_task);
    kSwitchContext(&prev_task->context, &task_to_run->context);
  }

  scheduler.processor_time = TASK_PROCESSOR_TIME;
  kUnlockForSystemData(prev_flag);
}

bool kScheduleInInterrupt(void) {
  const bool prev_flag = kLockForSystemData();
  TaskControlBlock* next_task = kGetNextTaskToRun();
  if (!next_task) {
    kUnlockForSystemData(prev_flag);
    return false;
  }

  // address for saved context in interrupt handlers.
  char* ist_context_addr =
      (char*)IST_START_ADDRESS + IST_SIZE - sizeof(Context);

  // Set to run next_task.
  TaskControlBlock* prev_task = scheduler.running_task;
  TaskControlBlock* task_to_run = scheduler.running_task = next_task;

  // If prev task is idle task, increase idle time.
  if (prev_task->flags & TASK_FLAG_IDLE) {
    scheduler.spend_processor_time_ind_idle_task += TASK_PROCESSOR_TIME;
  }

  // If prev task is to end, we don't need to save context of prev task (current
  // context).
  if (prev_task->flags & TASK_FLAG_END_TASK) {
    kAddListToTail(&scheduler.task_to_end_list, prev_task);
  } else {
    // Save ist context in prev_task.
    memcpy(&prev_task->context, ist_context_addr, sizeof(Context));
    kAddTaskToReadyList(prev_task);
  }
  kUnlockForSystemData(prev_flag);

  // Switch context by copying.
  memcpy(ist_context_addr, &task_to_run->context, sizeof(Context));

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

TaskControlBlock* kRemoveTaskFromReadyList(uint64 id) {
  uint64 tcb_offset = GET_TCB_OFFSET_FROM_ID(id);
  if (tcb_offset >= TASK_MAX_COUNT) {
    return nullptr;
  }

  TaskControlBlock* target = &tcb_pool_manager.tcb[tcb_offset];
  if (target->link.id != id) {
    return nullptr;
  }

  enum TaskPriority priority = GET_TASK_PRIORITY(target);
  target = kRemoveList(&scheduler.task_to_run_list[priority], id);
  return target;
}

bool kChangeTaskPriority(uint64 id, enum TaskPriority priority) {
  if (priority >= kTaskNumberOfPriority) {
    return false;
  }

  const bool prev_flag = kLockForSystemData();

  // When task is running task, only change priority.
  TaskControlBlock* target = scheduler.running_task;
  if (target->link.id == id) {
    SET_TASK_PRIORITY(target, priority);

    kUnlockForSystemData(prev_flag);
    return true;
  }

  // When task is in ready list, remove it from list and change priority, push
  // it to list.
  target = kRemoveTaskFromReadyList(id);
  if (target) {
    SET_TASK_PRIORITY(target, priority);
    kAddTaskToReadyList(target);

    kUnlockForSystemData(prev_flag);
    return true;
  }

  // If task is not in ready list, find it on TCB pool.
  target = kGetTCBInTCBPool(GET_TCB_OFFSET_FROM_ID(id));
  if (target) {
    SET_TASK_PRIORITY(target, priority);

    kUnlockForSystemData(prev_flag);
    return true;
  }

  kUnlockForSystemData(prev_flag);
  return false;
}

bool kEndTask(uint64 id) {
  const bool prev_flag = kLockForSystemData();
  TaskControlBlock* target = scheduler.running_task;
  // When need to end current running task.
  if (target->link.id == id) {
    target->flags |= TASK_FLAG_END_TASK;
    SET_TASK_PRIORITY(target, kTaskPriorityWait);

    kUnlockForSystemData(prev_flag);

    kSchedule();

    // Below codes are never executed.
    while (1)
      ;
  }
  // When need to find task in task_to_run_list.
  else {
    target = kRemoveTaskFromReadyList(id);
    if (target) {
      target->flags |= TASK_FLAG_END_TASK;
      SET_TASK_PRIORITY(target, kTaskPriorityWait);
      kAddListToTail(&scheduler.task_to_end_list, target);

      kUnlockForSystemData(prev_flag);
      return true;
    }

    // If tasks is not in ready list.
    target = kGetTCBInTCBPool(GET_TCB_OFFSET_FROM_ID(id));
    if (target) {
      target->flags |= TASK_FLAG_END_TASK;
      SET_TASK_PRIORITY(target, kTaskPriorityWait);
    }

    kUnlockForSystemData(prev_flag);
    return false;
  }
}

inline void kExitTask(void) { kEndTask(scheduler.running_task->link.id); }

size_t kGetReadyTaskCount(void) {
  const bool prev_flag = kLockForSystemData();
  size_t total_count = 0;
  for (int i = 0; i < kTaskNumberOfPriority; ++i) {
    total_count += kGetListCount(&scheduler.task_to_run_list[i]);
  }
  kUnlockForSystemData(prev_flag);
  return total_count;
}

inline size_t kGetTaskCount(void) {
  const bool prev_flag = kLockForSystemData();
  size_t ret =
      kGetReadyTaskCount() + kGetListCount(&scheduler.task_to_end_list) + 1;
  kUnlockForSystemData(prev_flag);
  return ret;
}

inline TaskControlBlock* kGetTCBInTCBPool(uint64 offset) {
  if (offset > TASK_MAX_COUNT) {
    return nullptr;
  }
  return &tcb_pool_manager.tcb[offset];
}

inline bool kIsTaskExist(uint64 id) {
  const TaskControlBlock* tcb = kGetTCBInTCBPool(GET_TCB_OFFSET_FROM_ID(id));
  return (tcb && IS_TASK_PRESENT(tcb));
}

inline uint64 kGetProcessorLoad(void) { return scheduler.processor_load; }

void kIdleTask(void) {
  uint64 last_measure_tick = 0, last_spend_tick_in_idle_task = 0;
  uint64 current_measure_tick, current_spend_tick_in_idle_task;
  while (1) {
    current_measure_tick = kGetTickCount();
    current_spend_tick_in_idle_task =
        scheduler.spend_processor_time_ind_idle_task;

    const uint64 tick_diff = current_measure_tick - last_measure_tick;
    const uint64 spend_tick_in_idle_diff =
        current_spend_tick_in_idle_task - last_spend_tick_in_idle_task;
    if (tick_diff) {
      scheduler.processor_load =
          100 - 100 * spend_tick_in_idle_diff / tick_diff;
    } else {
      scheduler.processor_load = 0;
    }

    last_measure_tick = current_measure_tick;
    last_spend_tick_in_idle_task = current_spend_tick_in_idle_task;

    kHaltProcessorByLoad();

    while (kGetListCount(&scheduler.task_to_end_list) > 0) {
      const bool prev_flag = kLockForSystemData();
      TaskControlBlock* task = kRemoveListFromHead(&scheduler.task_to_end_list);
      if (!task) {
        kUnlockForSystemData(prev_flag);
        break;
      }
      const uint64 id = task->link.id;
      kFreeTCB(id);
      kUnlockForSystemData(prev_flag);
    }

    kSchedule();
  }
}

inline void kHaltProcessor(void) {
  asm volatile(
      "hlt;"
      "hlt;");
}

void kHaltProcessorByLoad(void) {
  const uint64 processor_load = scheduler.processor_load;
  if (processor_load < 40) {
    kHaltProcessor();
    kHaltProcessor();
    kHaltProcessor();
  } else if (processor_load < 80) {
    kHaltProcessor();
    kHaltProcessor();
  } else if (processor_load < 95) {
    kHaltProcessor();
  }
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