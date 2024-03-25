#include "Console.h"
#include "ConsoleShell.h"
#include "Descriptor.h"
#include "Interrupt.h"
#include "Keyboard.h"
#include "Memory.h"
#include "PIC.h"
#include "PIT.h"
#include "Task.h"
#include "Types.h"

void Main(void) {
#define DO_STUCK_IF(x) while ((x))
  int cursor_x = 0, cursor_y = 10;

  kInitializeConsole(cursor_x, cursor_y);
  printf("Switch To IA-32e Mode Success~!!\n");
  printf("IA-32e C Language Kernel Start..............[Pass]\n");

  kGetCursor(&cursor_x, &cursor_y);
  printf("GDT Initialize And Switch For IA-32e Mode...[    ]");
  kInitializeGDTTableAndTSS();
  kLoadGDTR(GDTR_START_ADDRESS);
  kSetCursor(45, cursor_y++);
  printf("Pass\n");

  printf("TSS Segment Load............................[    ]");
  kLoadTR(GDT_TSS_SEGMENT);
  kSetCursor(45, cursor_y++);
  printf("Pass\n");

  printf("IDT Initialize..............................[    ]");
  kInitializeIDTTables();
  kLoadIDTR(IDTR_START_ADDRESS);
  kSetCursor(45, cursor_y++);
  printf("Pass\n");

  printf("Total RAM Size Check........................[    ]");
  kCalculateRamSize();
  kSetCursor(45, cursor_y++);
  printf("Pass], Size = %d MB\n", kGetRamSize());

  printf("TCB Pool And Scheduler Initialize...........[Pass]\n");
  ++cursor_y;
  kInitializeScheduler();
  kInitializePIT(MILLISEC_TO_COUNT(1), 1);

  printf("Keyboard Activate And Queue Initialize......[    ]");
  kSetCursor(45, cursor_y++);
  if (kInitializeKeyboard()) {
    printf("Pass\n");
  } else {
    printf("Fail\n");
    DO_STUCK_IF(1);
  }

  printf("PIC Controller And Interrupt Initialize.....[    ]");
  kInitializePIC();
  kMaskPICInterrupt(0);
  kEnableInterrupt();
  kSetCursor(45, cursor_y++);
  printf("Pass\n");

  kCreateTask(kTaskPriorityLowest | TASK_FLAG_THREAD | TASK_FLAG_SYSTEM |
                  TASK_FLAG_IDLE,
              0, 0, (uint64)kIdleTask);
  kStartConsoleShell();

  // We dont need "while(1);" here because "jmp $" in EntryPoint.s
#undef DO_STUCK_IF
}
