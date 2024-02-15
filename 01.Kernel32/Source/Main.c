#include "Types.h"

// Print string on (x, y).
// This function does not change attrubutes.
// @param x: 0 means left. (0 <= x < 80)
// @param y: 0 means top. (0 <= y < 25)
void kPrintString(const int x, const int y, const char* string);

// Set [0x100000, 0x600000) to zeros.
// @return false when fail to set.
bool kInitializeKernel64Area(void);

void Main() {
  kPrintString(0, 3, "C Language Kernel Started");
  bool success_to_initialize = kInitializeKernel64Area();
  if (success_to_initialize) {
    kPrintString(0, 4, "IA-32e Kernel Area Initilization Complete");
  } else {
    kPrintString(0, 4, "Failed to Initilize IA-32e Kernel Area");
  }
  while (1)
    ;
}

void kPrintString(const int x, const int y, const char* string) {
  Character* screen = (Character*)0xB8000 + 80 * y + x;
  for (const char* p = string; *p; ++p) {
    (*screen++).charactor = *p;
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