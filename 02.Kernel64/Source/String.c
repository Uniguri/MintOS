#include "String.h"

#include "Types.h"

inline void kPrintString(const int x, const int y, const char* string) {
  Character* screen = (Character*)0xB8000 + 80 * y + x;
  for (const char* p = string; *p; ++p) {
    screen->charactor = *p;
    ++screen;
  }
}

inline int strcmp(const char* s1, const char* s2) {
  while (*s1 && (*s1 == *s2)) {
    ++s1;
    ++s2;
  }
  return *(uint8*)s1 - *(uint8*)s2;
}

inline char* strcpy(char* s1, const char* s2) {
  char* p = s1;
  while (*s2) {
    *p++ = *s2++;
  }
  return s1;
}