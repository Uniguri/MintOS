#include "Descriptor.h"
#include "Interrupt.h"
#include "Keyboard.h"
#include "PIC.h"
#include "String.h"
#include "Types.h"

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

  kPrintString(0, 15, "Keyboard Activate And Queue Initialize......[    ]");
  if (kInitializeKeyboard()) {
    kPrintString(45, 15, "Pass");
  } else {
    kPrintString(45, 15, "Fail");
    DO_STUCK_IF(1);
  }

  kPrintString(0, 16, "PIC Controller And Interrupt Initialize.....[    ]");
  kInitializePIC();
  kMaskPICInterrupt(0);
  kEnableInterrupt();
  kPrintString(45, 16, "Pass");

  int location = 0;
  while (1) {
    KeyData key_data;
    if (kGetKeyFromKeyQueue(&key_data)) {
      if (key_data.flag == kFlagDown) {
        char ascii[2] = {key_data.ascii_code, 0};
        kPrintString(location++, 17, ascii);

        if (*ascii == '0') {
          *ascii /= 0;
        }
      }
    }
  }

  // We dont need "while(1);" here because "jmp $" in EntryPoint.s
#undef DO_STUCK_IF
}
