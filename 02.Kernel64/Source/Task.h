#ifndef MINTOS_TASK_H_
#define MINTOS_TASK_H_

#include "List.h"
#include "Macro.h"
#include "Types.h"

#define TASK_REGISTER_COUNT (5 + 19)
#define TASK_REGISTER_SIZE (8)
#define TASK_GS_OFFSET (0)
#define TASK_FS_OFFSET (1)
#define TASK_ES_OFFSET (2)
#define TASK_DS_OFFSET (3)
#define TASK_R15_OFFSET (4)
#define TASK_R14_OFFSET (5)
#define TASK_R13_OFFSET (6)
#define TASK_R12_OFFSET (7)
#define TASK_R11_OFFSET (8)
#define TASK_R10_OFFSET (9)
#define TASK_R9_OFFSET (10)
#define TASK_R8_OFFSET (11)
#define TASK_RSI_OFFSET (12)
#define TASK_RDI_OFFSET (13)
#define TASK_RDX_OFFSET (14)
#define TASK_RCX_OFFSET (15)
#define TASK_RBX_OFFSET (16)
#define TASK_RAX_OFFSET (17)
#define TASK_RBP_OFFSET (18)
#define TASK_RIP_OFFSET (19)
#define TASK_CS_OFFSET (20)
#define TASK_RFLAGS_OFFSET (21)
#define TASK_RSP_OFFSET (22)
#define TASK_SS_OFFSET (23)

#define TASK_ID_PRESENT (BIT(32llu))
#define IS_TASK_PRESENT(task) (IS_BIT_SET((task)->link.id, 32llu))

#define TASK_TCB_POOL_ADDRESS (BYTE_FROM_MB(8))
#define TASK_MAX_COUNT (1024)
#define GET_TCB_OFFSET_FROM_ID(id) ((id) & 0xFFFFFFFF)
#define TASK_STACK_POOL_ADDRESS \
  (TASK_TCB_POOL_ADDRESS + sizeof(TaskControlBlock) * TASK_MAX_COUNT)
#define TASK_STACK_SIZE (BYTE_FROM_KB(8))
#define TASK_INVALID_ID (0xFFFFFFFFFFFFFFFFu)
#define TASK_PROCESSOR_TIME (5)

#define GET_TASK_PRIORITY(task) ((task)->flags & 0xFF)
#define SET_TASK_PRIORITY(task, priority) \
  ((task)->flags = ((task)->flags & 0xFFFFFFFFFFFFFF00) | (priority))
enum TaskPriority {
  kTaskPriorityHighest = 0,
  kTaskPriorityHigh,
  kTaskPriorityMedium,
  kTaskPriorityLow,
  kTaskPriorityLowest,
  kTaskNumberOfPriority,
  kTaskPriorityWait = 0xFF,
};

#define TASK_FLAG_END_TASK (BIT(63llu))
#define TASK_FLAG_IDLE (BIT(59llu))

#pragma pack(push, 1)
typedef struct kContextStruct {
  uint64 gs;
  uint64 fs;
  uint64 es;
  uint64 ds;
  uint64 r15;
  uint64 r14;
  uint64 r13;
  uint64 r12;
  uint64 r11;
  uint64 r10;
  uint64 r9;
  uint64 r8;
  uint64 rsi;
  uint64 rdi;
  uint64 rdx;
  uint64 rcx;
  uint64 rbx;
  uint64 rax;
  uint64 rbp;
  uint64 rip;
  uint64 cs;
  uint64 rflags;
  uint64 rsp;
  uint64 ss;
} Context;
_Static_assert(sizeof(Context) == 8 * TASK_REGISTER_COUNT);

typedef struct kTaskControlBlockStruct {
  ListLink link;
  uint64 flags;

  union {
    Context context;
    uint64 reg_context[TASK_REGISTER_COUNT];
  };

  void* stack_addr;
  uint64 stack_size;
} TaskControlBlock;

typedef struct kTCBPollManagerStruct {
  TaskControlBlock* tcb;
  size_t max_count;
  size_t use_count;
  size_t alloacted_count;
} TCBPoolManager;

typedef struct kSchedulerStruct {
  TaskControlBlock* running_task;
  uint32 processor_time;

  List task_to_run_list[kTaskNumberOfPriority];
  List task_to_end_list;
  uint32 execute_count[kTaskNumberOfPriority];

  uint64 processor_load;
  uint64 spend_processor_time_ind_idle_task;
} Scheduler;
#pragma pack(pop)

// save current context on current_context and switch to next_context.
// Implement in TaskASM.asm
// @param current_context: pointer to save current_context.
// @param next_context: pointer to context to switch.
void kSwitchContext(Context* current_context_or_null, Context* next_context);

void kInitializeTCBPool(void);
TaskControlBlock* kAllocateTCB(void);
void kFreeTCB(uint64 id);
TaskControlBlock* kCreateTask(uint64 flags, uint64 entry_point_addr);
void kSetUpTask(TaskControlBlock* tcb, uint64 flags, uint64 entry_point_addr,
                void* stack_addr, uint64 stack_size);

void kInitializeScheduler(void);
// Set running task.
// @param task: task to be running task.
void kSetRunningTask(TaskControlBlock* task);
TaskControlBlock* kGetRunningTask(void);
TaskControlBlock* kGetNextTaskToRun(void);
// Add task to list containing tasks to run.
// @param task: task to add.
bool kAddTaskToReadyList(TaskControlBlock* task);
// Do scheduling.
void kSchedule(void);
// Do scheduling on interrupt. This function MUST be called
// when interrupt occurs.
bool kScheduleInInterrupt(void);
void kDecreaseProcessorTime(void);
// Check occupied time of current processor exceed given time.
// @return True if we need to change task.
bool kIsProcessorTimeExpired(void);
// Remove task from ready list(list containing tasks to run) and return it.
// @param id: id to remove and get.
// @return pointer to removed tcb from list containing tasks to run.
TaskControlBlock* kRemoveTaskFromReadyList(uint64 id);
bool kChangeTaskPriority(uint64 id, enum TaskPriority priority);
// End specific task.
// @param id: id of task to end. If you want to end present(alive) task, id's
// 32bit(present bit) must be set. Otherwise this function may cause General
// Protection fault.
// @return true if success.
bool kEndTask(uint64 id);
// End current running task.
void kExitTask(void);
// Get the number of tasks to run (tasks in list to run).
// @return number of tasks to run.
size_t kGetReadyTaskCount(void);
// Get the number of all tasks (tasks in list to run and end).
// @return the number of all tasks.
size_t kGetTaskCount(void);
// Get TCB at offset(index) on TCB pool.
// @param offset: offset(index) of TCB.
// @return valid pointer if success.
TaskControlBlock* kGetTCBInTCBPool(uint64 offset);
bool kIsTaskExist(uint64 id);
uint64 kGetProcessorLoad(void);

void kIdleTask(void);
void kHaltProcessor(void);
void kHaltProcessorByLoad(void);

#endif