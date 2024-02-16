#include "Types.h"

// Print string on (x, y).
// This function does not change attrubutes.
// @param x: 0 means left. (0 <= x < 80)
// @param y: 0 means top. (0 <= y < 25)
void kPrintString(const int x, const int y, const char* string);

void Main(void) {
  kPrintString(0, 10, "Switch To IA-32e Mode Success");
  kPrintString(0, 11, "IA-32e C Language Kernel Start..............[Pass]");

  // We dont need "while(1);" here because "jmp $" in EntryPoint.s
}

void kPrintString(const int x, const int y, const char* string) {
  Character* screen = (Character*)0xB8000 + 80 * y + x;
  for (const char* p = string; *p; ++p) {
    screen->charactor = *p;
    ++screen;
  }
}