#include "InterruptHandler.h"

#include "Console.h"
#include "HardDisk.h"
#include "Hardware.h"
#include "Keyboard.h"
#include "PIC.h"
#include "Task.h"
#include "Tick.h"

void kCommonExceptionHandler(int vector_number, uint64 error_code) {
#define DO_STUCT_IF(x) while ((x))
  char buffer[3] = {
      0,
  };

  buffer[0] = '0' + vector_number / 10;
  buffer[1] = '0' + vector_number % 10;

  kPrintStringXY(0, 0, "====================================================");
  kPrintStringXY(0, 1, "                 Exception Occur~!!!!               ");
  kPrintStringXY(0, 2, "                    Vector:                         ");
  kPrintStringXY(27, 2, buffer);
  kPrintStringXY(0, 3, "====================================================");

  DO_STUCT_IF(1);
#undef DO_STUCT_IF
}

void kCommonInterruptHandler(int vector_number) {
  char buffer[] = "[INT:  , ]";
  static int common_interrupt_count = 0;

  buffer[5] = '0' + vector_number / 10;
  buffer[6] = '0' + vector_number % 10;
  buffer[8] = '0' + common_interrupt_count;
  common_interrupt_count = (common_interrupt_count + 1) % 10;
  kPrintStringXY(70, 0, buffer);

  kSendEOIToPIC(vector_number - PIC_IRQ_START_VECTOR);
}

void kKeyboardHandler(int vector_number) {
  char buffer[] = "[INT:  , ]";
  static int keyboard_interrupt_count = 0;

  buffer[5] = '0' + vector_number / 10;
  buffer[6] = '0' + vector_number % 10;
  buffer[8] = '0' + keyboard_interrupt_count;
  keyboard_interrupt_count = (keyboard_interrupt_count + 1) % 10;
  kPrintStringXY(0, 0, buffer);

  if (kIsOutputBufferFull()) {
    uint8 scan_code = kGetKeyboardScanCode();
    kConvertScanCodeAndPushToQueue(scan_code);
  }

  kSendEOIToPIC(vector_number - PIC_IRQ_START_VECTOR);
}

void kTimerHandler(int vector_number) {
  char buffer[] = "[INT:  , ]";
  static int timer_interrupt_count = 0;

  buffer[5] = '0' + vector_number / 10;
  buffer[6] = '0' + vector_number % 10;
  buffer[8] = '0' + timer_interrupt_count;
  if (++timer_interrupt_count > 10) {
    timer_interrupt_count = 0;
  }
  kPrintStringXY(70, 0, buffer);

  kSendEOIToPIC(vector_number - PIC_IRQ_START_VECTOR);

  ++tick_count;

  kDecreaseProcessorTime();
  if (kIsProcessorTimeExpired()) {
    kScheduleInInterrupt();
  }
}

void kDeviceNotAvailableHandler(int vector_number) {
  char buffer[] = "[EXC:  , ]";
  static int fpu_interrupt_count = 0;

  buffer[5] = '0' + vector_number / 10;
  buffer[6] = '0' + vector_number % 10;
  buffer[8] = '0' + fpu_interrupt_count;
  if (++fpu_interrupt_count > 10) {
    fpu_interrupt_count = 0;
  }
  kPrintStringXY(0, 0, buffer);

  kClearTS();

  uint64 last_fpu_task_id = kGetLaskFPUUsedTaskID();
  TaskControlBlock* cur_task = kGetRunningTask();

  if (last_fpu_task_id == cur_task->id_link.id) {
    return;
  } else if (last_fpu_task_id != TASK_INVALID_ID) {
    TaskControlBlock* fpu_task =
        kGetTCBInTCBPool(GET_TCB_OFFSET_FROM_ID(last_fpu_task_id));
    if (fpu_task != nullptr && fpu_task->id_link.id == last_fpu_task_id) {
      kSaveFPUContext(fpu_task->fpu_context);
    }
  }

  if (cur_task->fpu_used == false) {
    kInitializeFPU();
    cur_task->fpu_used = true;
  } else {
    kLoadFPUContext(cur_task->fpu_context);
  }

  kSetLastFPUUsedTaskID(cur_task->id_link.id);
}

void kHDDHandler(int vector_number) {
  char buffer[] = "[INT:  , ]";
  static int hdd_interrupt_count = 0;

  buffer[5] = '0' + vector_number / 10;
  buffer[6] = '0' + vector_number % 10;
  buffer[8] = '0' + hdd_interrupt_count;
  if (++hdd_interrupt_count > 10) {
    hdd_interrupt_count = 0;
  }
  kPrintStringXY(70, 0, buffer);

  if (vector_number - PIC_IRQ_START_VECTOR == 14) {
    kSetHDDInterruptFlag(true, true);
  } else {
    kSetHDDInterruptFlag(false, true);
  }

  kSendEOIToPIC(vector_number - PIC_IRQ_START_VECTOR);
}