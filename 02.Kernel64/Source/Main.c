#include "Descriptor.h"
#include "Keyboard.h"
#include "Types.h"

// Print string on (x, y).
// This function does not change attrubutes.
// @param x: 0 means left. (0 <= x < 80)
// @param y: 0 means top. (0 <= y < 25)
void kPrintString(const int x, const int y, const char* string);

void Main(void) {
#define DO_STUCK_IF(x) while (x)
  kPrintString(0, 10, "Switch To IA-32e Mode Success~!!");
  kPrintString(0, 11, "IA-32e C Language Kernel Start..............[Pass]");

  kPrintString(0, 12, "GDT Initialize And Switch For IA-32e Mode...[    ]");
  kInitializeGDTTableAndTSS();
  kLoadGDTR(GDTR_START_ADDRESS);
  kPrintString(45, 12, "Pass");

  kPrintString(0, 13, "TSS Segment Load............................[    ]");
  kLoadTR(GDT_TSS_SEGMENT);
  kPrintString(45, 13, "Pass");

  kPrintString(0, 14, "IDT Initialize..............................[    ]");
  kInitializeIDTTables();
  kLoadIDTR(IDTR_START_ADDRESS);
  kPrintString(45, 14, "Pass");

  kPrintString(0, 15, "Keyboard Activate...........................[    ]");
  if (kActivateKeyboard() == TRUE) {
    kPrintString(45, 15, "Pass");
  } else {
    kPrintString(45, 15, "Fail");
    DO_STUCK_IF(1);
  }

  kClearOutputPortByte();

  int location = 0;
  while (1) {
    uint8 scan_code = 0;
    if (kIsOutputBufferFull()) {
      scan_code = kGetKeyboardScanCode();
    }

    uint8 ascii[2] = {
        0,
    };
    enum KeyFlag flag;
    if (kConvertScanCodeToAsciiCode(scan_code, &ascii[0], &flag)) {
      if (flag == kFlagDown) {
        kPrintString(location++, 16, ascii);

        if (ascii[0] == '0') {
          int temp = 100 / 0;
        }
      }
    }
  }

  // We dont need "while(1);" here because "jmp $" in EntryPoint.s
#undef DO_STUCK_IF
}

void kPrintString(const int x, const int y, const char* string) {
  Character* screen = (Character*)0xB8000 + 80 * y + x;
  for (const char* p = string; *p; ++p) {
    screen->charactor = *p;
    ++screen;
  }
}