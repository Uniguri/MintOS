#include "Tick.h"

volatile uint64 tick_count = 0;

inline uint64 kGetTickCount(void) { return tick_count; }