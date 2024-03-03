#ifndef MINTOS_TASK_H_
#define MINTOS_TASK_H_

#include "List.h"
#include "Macro.h"
#include "Types.h"

#define TASK_REGISTER_COUNT (24)
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

#define TASK_TCB_POOL_ADDRESS (BYTE_FROM_MB(8))
#define TASK_MAX_COUNT (1024)
#define TASK_STACK_POOL_ADDRESS \
  (TASK_TCB_POOL_ADDRESS + sizeof(TaskControlBlock) + TASK_MAX_COUNT)
#define TASK_STACK_SIZE (BYTE_FROM_KB(8))
#define TASK_INVALID_ID (0xFFFFFFFFFFFFFFFFu)
#define TASK_PROCESSOR_TIME (5)

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
  TaskControlBlock* start_addr;
  size_t max_count;
  size_t use_count;
  size_t alloacted_count;
} TCBPoolManager;

typedef struct kSchedulerStruct {
  TaskControlBlock* running_task;
  uint32 processor_time;
  List ready_list;
} Scheduler;
#pragma pack(pop)

void kInitializeTCBPool(void);
TaskControlBlock* kAllocateTCB(void);
void kFreeTCB(uint64 id);
TaskControlBlock* kCreateTask(uint64 flags, uint64 entry_point_addr);
void kSetUpTask(TaskControlBlock* tcb, uint64 flags, uint64 entry_point_addr,
                void* stack_addr, uint64 stack_size);

void kInitializeScheduler(void);
void kSetRunningTask(TaskControlBlock* task);
TaskControlBlock* kGetRunningTask(void);
TaskControlBlock* kGetNextTaskToRun(void);
void kAddTaskToReadyList(TaskControlBlock* task);
void kSchedule(void);
bool kScheduleInInterrupt(void);
void kDecreaseProcessorTime(void);
bool kIsProcessorTimeExpired(void);

void kSwitchContext(Context* current_context, Context* next_context);

#endif