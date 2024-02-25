#include "Interrupt.h"

#include "Types.h"

void kEnableInterrupt(void) { asm volatile("sti;" ::); }

void kDisableInterrupt(void) { asm volatile("cli;" ::); }

uint64 kGetRFlags(void) {
  uint64 rflag = 0;
  asm volatile(
      "pushfq;"
      "pop %[rflag];"
      : [rflag] "=r"(rflag)
      :);
  return rflag;
}