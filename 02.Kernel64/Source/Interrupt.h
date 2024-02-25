#ifndef MINTOS_INTERRUPT_H_
#define MINTOS_INTERRUPT_H_

#include "Types.h"

// Enable interrupt via sti.
void kEnableInterrupt(void);

// Disable interrupt via cli.
void kDisableInterrupt(void);

// Get RFlags
// @return value of RFLAGS
uint64 kGetRFlags(void);

#endif