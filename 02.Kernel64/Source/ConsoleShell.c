#include "ConsoleShell.h"

#include "Console.h"
#include "Hardware.h"
#include "Interrupt.h"
#include "Keyboard.h"
#include "Macro.h"
#include "Memory.h"
#include "PIT.h"
#include "RTC.h"
#include "String.h"
#include "Task.h"

ShellCommandEntry command_table[] = {
    {"help", "Show Help", kHelp},
    {"clear", "Clear Screen", kClear},
    {"totalram", "Show Total RAM Size", kShowRamSize},
    {"settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)",
     kSetTimer},
    {"wait", "Wait ms Using PIT, ex)wait 100(ms)", kWaitUsingPIT},
    {"rdtsc", "Read Time Stamp Counter", kPrintTimeStampCounter},
    {"date", "Show Date", kShowDate},
    {"time", "Show Time", kShowTime},
    {"createtask", "Create Task", kCreateTestTask},
};

void kStartConsoleShell(void) {
  char command_buffer[CONSOLE_BUFFER_SIZE] = {
      0,
  };
  int command_buffer_idx = 0;

  printf(CONSOLE_SHELL_PROMPT_MESSAGE);

  while (1) {
    uint8 key = getch();

    if (key == (uint8)kBacksapce) {
      int x, y;
      kGetCursor(&x, &y);
      if (x > 7) {
        kPrintStringXY(x - 1, y, " ");
        kSetCursor(x - 1, y);

        command_buffer[command_buffer_idx--] = 0;
      }
    } else if (key == (uint8)kEnter) {
      printf("\n");

      if (command_buffer_idx > 0) {
        command_buffer[command_buffer_idx] = 0;
        kExecuteCommand(command_buffer);
      }

      printf(CONSOLE_SHELL_PROMPT_MESSAGE);
      memset(command_buffer, 0, CONSOLE_BUFFER_SIZE);
      command_buffer_idx = 0;
    } else if (key == (uint8)kLshift || key == (uint8)kRshift ||
               key == (uint8)kCapslock || key == (uint8)kNumlock ||
               key == (uint8)kScrolllock) {
      ;
    } else {
      if (key == (uint8)kTab) {
        key = ' ';
      }

      if (command_buffer_idx < CONSOLE_BUFFER_SIZE) {
        command_buffer[command_buffer_idx++] = key;
        printf("%c", key);
      }
    }
  }
}

void kExecuteCommand(const char* command_buffer) {
  size_t command_buffer_length = strlen(command_buffer);
  const char* arguments = strchr(command_buffer, ' ');
  size_t space_index = 0;
  if (arguments) {
    space_index = arguments - command_buffer;
  } else {
    space_index = command_buffer_length;
  }

  const size_t number_of_command =
      sizeof(command_table) / sizeof(ShellCommandEntry);
  for (size_t i = 0; i < number_of_command; ++i) {
    const char* command = command_table[i].command;
    if (!memcmp(command_buffer, command, space_index)) {
      return command_table[i].function(arguments + 1);
    }
  }

  printf("\"%s\" is not found.\n", command_buffer);
}

void kInitializeParameter(ParameterList* parameter_list,
                          const char* parameter) {
  parameter_list->buffer = parameter;
  parameter_list->length = strlen(parameter);
  parameter_list->current_position = 0;
}

int kGetNextParameter(ParameterList* parameter_list, char* parameter) {
  if (parameter_list->length <= parameter_list->current_position) {
    return 0;
  }

  const char* param_buf = parameter_list->buffer;
  const size_t cur_pos = parameter_list->current_position;
  const char* cur_buf = param_buf + cur_pos;
  int64 space_idx_of_cur_buf = strchr(cur_buf, ' ') - param_buf;
  if (space_idx_of_cur_buf <= 0) {
    const size_t param_length = parameter_list->length;
    if (*cur_buf) {
      space_idx_of_cur_buf = param_length;
    } else {
      return 0;
    }
  }

  memcpy(parameter, cur_buf, space_idx_of_cur_buf);
  const size_t param_len = space_idx_of_cur_buf - cur_pos;
  parameter[param_len] = 0;

  parameter_list->current_position += param_len + 1;
  return param_len;
}

#define MAKE_LIST_AND_PARAM(param_buf) \
  char param[CONSOLE_BUFFER_SIZE];     \
  ParameterList list;                  \
  kInitializeParameter(&list, param_buf)

#define GET_NEXT_PARAM() kGetNextParameter(&list, param)

void kHelp(const char* parameter_buffer) {
  printf("=========================================================\n");
  printf("                    MINT64 Shell Help                    \n");
  printf("=========================================================\n");

  const size_t number_of_command =
      sizeof(command_table) / sizeof(ShellCommandEntry);

  size_t max_command_length = 0;
  for (size_t i = 0; i < number_of_command; ++i) {
    size_t command_length = strlen(command_table[i].command);
    if (max_command_length < command_length) {
      max_command_length = command_length;
    }
  }

  for (size_t i = 0; i < number_of_command; ++i) {
    printf("%s", command_table[i].command);
    int cursor_x, cursor_y;
    kGetCursor(&cursor_x, &cursor_y);
    kSetCursor(max_command_length, cursor_y);
    printf("  - %s\n", command_table[i].help);
  }
}
void kClear(const char* parameter_buffer) {
  kClearScreen();
  kSetCursor(0, 1);
}

void kShowRamSize(const char* parameter_buffer) {
  printf("Total RAM size = %d MB.\n", kGetRamSize());
}

void kSetTimer(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  if (!GET_NEXT_PARAM()) {
    printf(
        "Usage: settimer [ms in decimal; 0<=ms<=65535] [is periodic; 1 or "
        "0]\n");
    return;
  }
  uint64 val = Uint64FromDecimalString(param);

  if (!GET_NEXT_PARAM()) {
    printf("Usage: settimer [ms in decimal] [is periodic; 1 or 0]\n");
    return;
  }
  bool periodic = (bool)Int32FromDecimalString(param);

  kInitializePIT(MILLISEC_TO_COUNT(val), periodic);
}

void kWaitUsingPIT(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  if (!GET_NEXT_PARAM()) {
    printf("Usage: wait [ms in decimal]\n");
    return;
  }
  uint64 ms = Uint64FromDecimalString(param);

  kDisableInterrupt();
  for (uint64 i = 0; i < ms / 30; ++i) {
    kWaitUsingDirectPIT(MILLISEC_TO_COUNT(30));
  }
  kWaitUsingDirectPIT(MILLISEC_TO_COUNT(ms % 30));
  kEnableInterrupt();

  kInitializePIT(MILLISEC_TO_COUNT(1), true);
}

void kPrintTimeStampCounter(const char* parameter_buffer) {
  printf("Time Stamp Counter = %q\n", kReadTSC());
}

void kShowDate(const char* parameter_buffer) {
  uint8 sec, min, hour;
  uint8 day_of_week, day_of_month, month;
  uint16 year;

  kReadRTCTime(&hour, &min, &sec);
  kReadRTCDate(&year, &month, &day_of_month, &day_of_week);

  printf("%d/%d/%d %s\n", year, month, day_of_month,
         kConvertDayOfWeekToString(day_of_week));
}

void kShowTime(const char* parameter_buffer) {
  uint8 sec, min, hour;
  uint8 day_of_week, day_of_month, month;
  uint16 year;

  kReadRTCTime(&hour, &min, &sec);
  kReadRTCDate(&year, &month, &day_of_month, &day_of_week);

  printf("%d:%d:%d\n", hour, min, sec);
}

void kTestTask1(void) {
  Character* screen = (Character*)CONSOLE_VIDEO_MEMORY_ADDRESS;
  TaskControlBlock* running_task = kGetRunningTask();

  uint8 data = 0;
  uint32 margin = (running_task->link.id & 0xFFFFFFFF) % 10;
  uint32 i = 0, x = 0, y = 0;
  while (1) {
    switch (i) {
      case 0:
        if (++x >= (CONSOLE_WIDTH - margin)) {
          i = 1;
        }
        break;
      case 1:
        if (++y >= (CONSOLE_WIDTH - margin)) {
          i = 2;
        }
        break;
      case 2:
        if (--x < margin) {
          i = 3;
        }
        break;
      case 3:
        if (--y < margin) {
          i = 0;
        }
        break;
    }

    screen[y * CONSOLE_WIDTH + x].charactor = data;
    screen[y * CONSOLE_WIDTH + x].attribute = data & 0x0F;
    ++data;

    kSchedule();
  }
}

void kTestTask2(void) {
  Character* screen = (Character*)CONSOLE_VIDEO_MEMORY_ADDRESS;
  TaskControlBlock* running_task = kGetRunningTask();
  const char data[4] = {'-', '\\', '|', '/'};

  uint32 offset = (running_task->link.id & 0xFFFFFFFF) * 2;
  offset = CONSOLE_WIDTH * CONSOLE_HEIGHT -
           (offset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

  uint32 i = 0;
  while (1) {
    screen[offset].charactor = data[i % 4];
    screen[offset].attribute = (offset % 15) + 1;
    ++i;

    kSchedule();
  }
}

void kCreateTestTask(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  if (!GET_NEXT_PARAM()) {
    printf("Usage: createtask [type; 1 or 2] [count]\n");
    return;
  }
  uint64 type = Int64FromDecimalString(param);

  if (!GET_NEXT_PARAM()) {
    printf("Usage: createtask [type; 1 or 2] [count]\n");
    return;
  }
  uint64 count = Int64FromDecimalString(param);

  switch (type) {
    case 1: {
      int i;
      for (i = 0; i < count; ++i) {
        if (!kCreateTask(0, (uint64)kTestTask1)) {
          break;
        }
      }
      break;
    }
    case 2: {
      int i;
      for (i = 0; i < count; ++i) {
        if (!kCreateTask(0, (uint64)kTestTask2)) {
          break;
        }
      }
      break;
    }
    default:
      printf("Invalid type\n");
      break;
  }
}

#undef MAKE_LIST_AND_PARAM
#undef GET_NEXT_PARAM