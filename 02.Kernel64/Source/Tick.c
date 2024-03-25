#include "Tick.h"

#include "Task.h"

volatile uint64 tick_count = 0;

inline uint64 kGetTickCount(void) { return tick_count; }

void kSleep(uint64 millisecond) {
  uint64 last_tick_count = tick_count;
  while (tick_count - last_tick_count <= millisecond) {
    kSchedule();
  }
}