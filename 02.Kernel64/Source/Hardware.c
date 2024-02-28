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

inline void kSetPortByte(uint16 port, uint8 data) {
  uint16 p = port, d = data;
  asm volatile(
      // Set data on port.
      "out dx, al\n"
      :
      : [port] "d"((uint64)port), [data] "a"(data));
}

inline uint64 kReadTSC(void) {
  uint32 high, low;
  asm volatile("rdtsc;" : "=a"(low), "=d"(high) :);
  return ((uint64)high << 32) | low;
}