#include "Console.h"

#include <stdarg.h>

#include "Hardware.h"
#include "Keyboard.h"
#include "Memory.h"
#include "String.h"
#include "Task.h"
#include "Types.h"

ConsoleManager console_manager = {
    0,
};

inline void kInitializeConsole(int x, int y) {
  memset(&console_manager, 0, sizeof(ConsoleManager));
  kSetCursor(x, y);
}

void kSetCursor(int x, int y) {
  int linear_val = x + y * CONSOLE_WIDTH;

  kSetPortByte(VGA_PORT_INDEX, VGA_INDEX_UPPER_CURSOR);
  kSetPortByte(VGA_PORT_DATA, linear_val >> 8);

  kSetPortByte(VGA_PORT_INDEX, VGA_INDEX_LOWER_CURSOR);
  kSetPortByte(VGA_PORT_DATA, linear_val & 0xFF);

  console_manager.current_print_offset = linear_val;
}

inline void kGetCursor(int* x, int* y) {
  const int linear_val = console_manager.current_print_offset;
  *x = linear_val % CONSOLE_WIDTH;
  *y = linear_val / CONSOLE_WIDTH;
}

void printf(const char* format, ...) {
  va_list va;
  char buffer[0x100];

  va_start(va, format);
  vsnprintf(buffer, 0x100, format, va);
  va_end(va);

  int next_print_offset = kConsolePrintString(buffer);
  kSetCursor(next_print_offset % CONSOLE_WIDTH,
             next_print_offset / CONSOLE_WIDTH);
}

int kConsolePrintString(const char* buffer) {
  Character* screen = (Character*)CONSOLE_VIDEO_MEMORY_ADDRESS;

  int print_offset = console_manager.current_print_offset;
  size_t length = strlen(buffer);
  for (size_t i = 0; i < length; ++i) {
    if (buffer[i] == '\n') {
      print_offset += (CONSOLE_WIDTH - (print_offset % CONSOLE_WIDTH));
    } else if (buffer[i] == '\t') {
      print_offset += (8 - (print_offset % 8));
    } else {
      screen[print_offset].charactor = buffer[i];
      screen[print_offset].attribute = CONSOLE_DEFAULT_TEXT_COLOR;
      ++print_offset;
    }
  }
  if (print_offset >= (CONSOLE_HEIGHT * CONSOLE_WIDTH)) {
    memcpy(CONSOLE_VIDEO_MEMORY_ADDRESS,
           CONSOLE_VIDEO_MEMORY_ADDRESS + CONSOLE_WIDTH * sizeof(Character),
           (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH * sizeof(Character));
    for (size_t j = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
         j < CONSOLE_HEIGHT * CONSOLE_WIDTH; ++j) {
      screen[j].charactor = ' ';
      screen[j].attribute = CONSOLE_DEFAULT_TEXT_COLOR;
    }

    print_offset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
  }

  return print_offset;
}

void kClearScreen(void) {
  Character* screen = (Character*)CONSOLE_VIDEO_MEMORY_ADDRESS;
  for (size_t i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; ++i) {
    screen[i].charactor = ' ';
    screen[i].attribute = CONSOLE_DEFAULT_TEXT_COLOR;
  }
}

uint8 getch(void) {
  while (1) {
    KeyData key_data;
    while (!kGetKeyFromKeyQueue(&key_data)) {
      // kSchedule();
    }
    if (key_data.flag == kFlagDown) {
      return key_data.ascii_code;
    }
  }
}

void kPrintStringXY(int x, int y, const char* s) {
  Character* screen =
      (Character*)CONSOLE_VIDEO_MEMORY_ADDRESS + CONSOLE_WIDTH * y + x;
  for (const char* p = s; *p; ++p) {
    screen->charactor = *p;
    ++screen;
  }
}