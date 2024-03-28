#ifndef MINTOS_Hardware_H_
#define MINTOS_Hardware_H_

#include "Types.h"

// @param port: port number of register.
// @return data of the register
uint8 kGetPortByte(uint16 port);

// @param port: port number of register.
// @param data: data to set on port.
void kSetPortByte(uint16 port, uint8 data);

uint64 kReadTSC(void);

void kInitializeFPU(void);
void kSaveFPUContext(void* fpu_context);
void kLoadFPUContext(void* fpu_context);

void kSetTS(void);
void kClearTS(void);
#endif