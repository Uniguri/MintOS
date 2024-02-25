#include "Memory.h"

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