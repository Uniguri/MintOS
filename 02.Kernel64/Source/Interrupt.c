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

inline void kSetInterruptFlag(bool interrupt_status) {
  if (interrupt_status) {
    kEnableInterrupt();
  } else {
    kDisableInterrupt();
  }
}