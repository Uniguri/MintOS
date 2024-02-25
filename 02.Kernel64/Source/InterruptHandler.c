#include "InterruptHandler.h"

#include "PIC.h"
#include "String.h"

void kCommonExceptionHandler(int vector_number, uint64 error_code) {
#define DO_STUCT_IF(x) while (x)
  char buffer[3] = {
      0,
  };

  buffer[0] = '0' + vector_number / 10;
  buffer[1] = '0' + vector_number % 10;

  kPrintString(0, 0, "====================================================");
  kPrintString(0, 1, "                 Exception Occur~!!!!               ");
  kPrintString(0, 2, "                    Vector:                         ");
  kPrintString(27, 2, buffer);
  kPrintString(0, 3, "====================================================");

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
  kPrintString(70, 0, buffer);

  kSendEOIToPIC(vector_number - PIC_IRQ_START_VECTOR);
}

void kKeyboardHandler(int vector_number) {
  char buffer[] = "[INT:  , ]";
  static int keyboard_interrupt_count = 0;

  buffer[5] = '0' + vector_number / 10;
  buffer[6] = '0' + vector_number % 10;
  buffer[8] = '0' + keyboard_interrupt_count;
  keyboard_interrupt_count = (keyboard_interrupt_count + 1) % 10;
  kPrintString(0, 0, buffer);

  kSendEOIToPIC(vector_number - PIC_IRQ_START_VECTOR);
}
