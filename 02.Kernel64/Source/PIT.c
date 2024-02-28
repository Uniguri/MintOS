#include "PIT.h"

#include "Hardware.h"

inline void kInitializePIT(uint16 count, bool periodic) {
  kSetPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);
  if (periodic) {
    kSetPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);
  }

  kSetPortByte(PIT_PORT_COUNTER0, count & 0xFF);
  kSetPortByte(PIT_PORT_COUNTER0, count >> 8);
}

inline uint16 kReadCounter0(void) {
  kSetPortByte(PIT_PORT_CONTROL, PIT_CONTROL_LATCH);

  const uint8 low_byte = kGetPortByte(PIT_CONTROL_COUNTER0);
  const uint8 high_byte = kGetPortByte(PIT_CONTROL_COUNTER0);
  return (high_byte << 8) | low_byte;
}

inline void kWaitUsingDirectPIT(uint16 count) {
  kInitializePIT(0, true);

  const uint16 last_counter0 = kReadCounter0();
  uint16 current_counter0;
  do {
    current_counter0 = kReadCounter0();
  } while ((last_counter0 - current_counter0) & 0xFFFF < count);
}