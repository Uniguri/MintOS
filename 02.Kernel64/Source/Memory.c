#include "Memory.h"

#include "Macro.h"
#include "Types.h"

static size_t ram_size;

size_t kCalculateRamSize(void) {
#define TEST_VALUE (0xDEADBEEFCAFEBEBEu)
  // First, Check 4MB ~ 64MB in 1MB increments.
  uint64* addr = (uint64*)BYTE_FROM_MB(4);
  uint64 prev_val;
  while (addr < (uint64*)BYTE_FROM_MB(64)) {
    prev_val = *addr;
    *addr = TEST_VALUE;
    if (*addr != TEST_VALUE) {
      break;
    }
    *addr = prev_val;
    addr += BYTE_FROM_MB(1) / 8;
  }

  // Then, Check 64MB ~ 1GB in 16MB increments.
  while (addr < (uint64*)BYTE_FROM_GB(1)) {
    prev_val = *addr;
    *addr = TEST_VALUE;
    if (*addr != TEST_VALUE) {
      break;
    }
    *addr = prev_val;
    addr += BYTE_FROM_MB(16) / 8;
  }

  // Then, Check 1GB ~ 1TB in GB increments.
  while (addr < (uint64*)BYTE_FROM_TB(1)) {
    prev_val = *addr;
    *addr = TEST_VALUE;
    if (*addr != TEST_VALUE) {
      break;
    }
    *addr = prev_val;
    addr += BYTE_FROM_GB(1) / 8;
  }
#undef TEST_VALUE

  ram_size = MB_FROM_BYTE((size_t)addr);
}

inline size_t kGetRamSize(void) { return ram_size; }

void* memset(void* ptr, int val, size_t num) {
  uint8* p = (uint8*)ptr;
  for (size_t i = 0; i < num; ++i) {
    *p++ = (uint8)val;
  }
  return ptr;
}

void* memcpy(void* dest, const void* src, size_t count) {
  uint8* p = (uint8*)dest;
  for (size_t i = 0; i < count; ++i) {
    *p++ = *(uint8*)src++;
  }
  return dest;
}

int memcmp(const void* buf1, const void* buf2, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    int diff = *(uint8*)buf1++ - *(uint8*)buf2++;
    if (diff) {
      return diff;
    }
  }
  return 0;
}