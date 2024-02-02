#include "Types.h"

void kPrintString(int x, int y, const char* string);

void Main() {
  kPrintString(0, 3, "C Language Kernel Started");
  while (1)
    ;
}

void kPrintString(int x, int y, const char* string) {
  Character* screen = (Character*)0xB8000;
  for (const char* p = string; *p; ++p) {
    (*screen++).charactor = *p;
  }
}