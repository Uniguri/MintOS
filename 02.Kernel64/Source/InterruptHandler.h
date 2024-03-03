#ifndef MINTOS_INTERRUPTHANDLER_H_
#define MINTOS_INTERRUPTHANDLER_H_

#include "Types.h"

void kCommonExceptionHandler(int vector_number, uint64 error_code);
void kCommonInterruptHandler(int vector_number);
void kKeyboardHandler(int vector_number);
void kTimerHandler(int vector_number);

#endif