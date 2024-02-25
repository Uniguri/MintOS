#ifndef MINTOS_MEMORY_H_
#define MINTOS_MEMORY_H_

#include "Types.h"

void *memset(void *ptr, int val, size_t num);
void *memcpy(void *dest, const void *src, size_t count);
int memcmp(const void *buf1, const void *buf2, size_t count);

#endif