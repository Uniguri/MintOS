#include "Synchronization.h"

#include "Interrupt.h"
#include "Task.h"

inline bool kLockForSystemData(void) { return kSetInterruptFlag(false); }

inline void kUnlockForSystemData(bool interrupt_flag) {
  kSetInterruptFlag(interrupt_flag);
}

void kInitializeMutex(Mutex* mutex) {
  mutex->is_lock = false;
  mutex->lock_count = 0;
  mutex->task_id = TASK_INVALID_ID;
}

inline bool kCompareAndSet(volatile uint8* dst, uint8 expected,
                           uint8 new_value) {
  // Use __atomic_compare_exchange_n instead of using inline asm.
  // asm volatile(
  //     "lock cmpxchg %[dst], %[new_val];"
  //     // Save result(ZF) to expected.
  //     "setz al"
  //     : [dst] "+m"(*dst), [expected] "+a"(expected)
  //     : [new_val] "r"(new_value)
  //     : "memory");
  // // expected store result(ZF).
  // return (bool)expected;
  return __atomic_compare_exchange_n(dst, &expected, new_value, false,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

void kLockMutex(Mutex* mutex) {
  if (!kCompareAndSet(&mutex->is_lock, false, true)) {
    if (mutex->task_id == kGetRunningTask()->link.id) {
      ++mutex->lock_count;
      return;
    }

    while (!kCompareAndSet(&mutex->is_lock, false, true)) {
      kSchedule();
    }
  }

  mutex->lock_count = 1;
  mutex->task_id = kGetRunningTask()->link.id;
}

void kUnlockMutex(Mutex* mutex) {
  if (!mutex->is_lock || mutex->task_id != kGetRunningTask()->link.id) {
    return;
  }

  if (mutex->lock_count > 1) {
    --mutex->lock_count;
    return;
  }

  mutex->task_id = TASK_INVALID_ID;
  mutex->lock_count = 0;
  mutex->is_lock = false;
}