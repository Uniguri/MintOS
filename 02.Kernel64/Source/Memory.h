#ifndef MINTOS_MEMORY_H_
#define MINTOS_MEMORY_H_

#include "Types.h"

// Calculate RAM size and return it.
// @return RAM size (MB); e.g. 2 when RAM is 2MB, 8192 when 8GB.
size_t kCalculateRamSize(void);

// Get RAM size.
// @return RAM size (MB); e.g. 2 when RAM is 2MB, 8192 when 8GB.
size_t kGetRamSize(void);

void *memset(void *ptr, int val, size_t num);
void *memcpy(void *dest, const void *src, size_t count);
int memcmp(const void *buf1, const void *buf2, size_t count);

#endif