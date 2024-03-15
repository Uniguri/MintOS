#ifndef MINTOS_SYNCHRONIZATION_H_
#define MINTOS_SYNCHRONIZATION_H_

#include "Types.h"

#pragma pack(push, 1)
typedef struct kMutexStruct {
  volatile uint64 task_id;
  volatile uint32 lock_count;
  volatile bool is_lock;
  uint8 padding[3];
} Mutex;
#pragma pack(pop)

bool kLockForSystemData(void);

void kUnlockForSystemData(bool interrupt_flag);

// Set *dst to new_value when *dst == expected.
// @param dst: pointer to compare and set.
// @param expected: expected value of *dst.
// @param new_value: new_value of *dst when *dst == expected.
// @return True when *dst == expected and so set *des to new_value.
bool kCompareAndSet(volatile uint8* dst, uint8 expected, uint8 new_value);

void kInitializeMutex(Mutex* mutex);

void kLockMutex(Mutex* mutex);

void kUnlockMutex(Mutex* mutex);

#endif