#include "Hardware.h"

#include "Types.h"

inline uint8 kGetPortByte(uint16 port) {
  uint64 data = 0;
  asm volatile(
      // Read data from port
      "in al, dx\n"
      : [data] "=a"(data)
      : [port] "d"(port));
  return (uint8)(data & 0xFF);
}

inline uint16 kGetPortWord(uint16 port) {
  uint64 data = 0;
  asm volatile(
      // Read data from port
      "in ax, dx\n"
      : [data] "=a"(data)
      : [port] "d"(port));
  return (uint16)(data & 0xFFFF);
}

inline void kSetPortByte(uint16 port, uint8 data) {
  asm volatile(
      // Set data on port.
      "out dx, al\n"
      :
      : [port] "d"((uint64)port), [data] "a"(data));
}

inline void kSetPortWord(uint16 port, uint16 data) {
  asm volatile(
      // Set data on port.
      "out dx, ax\n"
      :
      : [port] "d"((uint64)port), [data] "a"(data));
}

inline uint64 kReadTSC(void) {
  uint32 high, low;
  asm volatile("rdtsc;" : "=a"(low), "=d"(high) :);
  return ((uint64)high << 32) | low;
}

inline void kInitializeFPU(void) { asm volatile("finit;"); }

inline void kSaveFPUContext(void* fpu_context) {
  asm volatile("fxsave [%[fpu_ctx]];" : : [fpu_ctx] "D"(fpu_context));
}

inline void kLoadFPUContext(void* fpu_context) {
  asm volatile("fxrstor [%[fpu_ctx]]" ::[fpu_ctx] "D"(fpu_context));
}

inline void kSetTS(void) {
  asm volatile(
      "mov rax, cr0;"
      "or rax, 0x08;"
      "mov cr0, rax;");
}

inline void kClearTS(void) { asm volatile("clts"); }