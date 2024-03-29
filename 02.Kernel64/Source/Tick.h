#ifndef MINTOS_TICK_H_
#define MINTOS_TICK_H_

#include "Types.h"

extern volatile uint64 tick_count;

uint64 kGetTickCount(void);

void kSleep(uint64 millisecond);

#endif