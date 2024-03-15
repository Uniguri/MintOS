#include "Interrupt.h"

#include "Macro.h"
#include "Types.h"

inline void kEnableInterrupt(void) { asm volatile("sti;" ::); }

inline void kDisableInterrupt(void) { asm volatile("cli;" ::); }

inline uint64 kGetRFlags(void) {
  uint64 rflag = 0;
  asm volatile(
      "pushfq;"
      "pop %[rflag];"
      : [rflag] "=r"(rflag)
      :);
  return rflag;
}

inline bool kIsInterruptEnabled(void) { return IS_BIT_SET(kGetRFlags(), 9); }

inline bool kSetInterruptFlag(bool interrupt_status) {
  uint64 before_flag = kGetRFlags();
  if (interrupt_status) {
    kEnableInterrupt();
  } else {
    kDisableInterrupt();
  }
  return IS_BIT_SET(before_flag, 9);
}