#include "ModeSwitch.h"
#include "Page.h"
#include "Types.h"

#define MAKE_STUCK(x) while (x)

// Print string on (x, y).
// This function does not change attrubutes.
// @param x: 0 means left. (0 <= x < 80)
// @param y: 0 means top. (0 <= y < 25)
void kPrintString(const int x, const int y, const char* string);

// Set [0x100000, 0x600000) of memory to zeros.
// @return false when fail to set.
bool kInitializeKernel64Area(void);

void Main() {
  kPrintString(0, 3, "C Language Kernel Start.....................[Pass]");
  kPrintString(0, 4, "IA-32e Kernel Area Initialize...............[    ]");
  if (!kInitializeKernel64Area()) {
    kPrintString(45, 4, "Fail");
    MAKE_STUCK(1);
  }
  kPrintString(45, 4, "Pass");

  kPrintString(0, 5, "IA-32e Page Tables Initialize...............[    ]");
  kInitializePageTables();
  kPrintString(45, 5, "Pass");
  {
    uint32 eax = 0, ebx = 0, ecx = 0, edx = 0;
    kReadCPUID(0x80000001, &eax, &ebx, &ecx, &edx);
    kPrintString(0, 6, "64bit Mode Support Check....................[    ]");
    if (!(edx & (1 << 29))) {
      kPrintString(45, 6, "Fail");
      MAKE_STUCK(1);
    }
    kPrintString(45, 6, "Pass");
  }
  MAKE_STUCK(1);
}

void kPrintString(const int x, const int y, const char* string) {
  Character* screen = (Character*)0xB8000 + 80 * y + x;
  for (const char* p = string; *p; ++p) {
    screen->charactor = *p;
    ++screen;
  }
}

bool kInitializeKernel64Area(void) {
  for (uint32* current_address = (uint32*)0x100000;
       current_address < (uint32*)0x600000; ++current_address) {
    *current_address = 0;
    // If *current_address is not zero, an error occurs and return false.
    if (*current_address) {
      return false;
    }
  }

  return true;
}