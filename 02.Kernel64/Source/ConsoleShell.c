#include "ConsoleShell.h"

#include "Console.h"
#include "Keyboard.h"
#include "Memory.h"
#include "String.h"

ShellCommandEntry command_table[] = {
    {"help", "Show Help", kHelp},
    {"clear", "Clear Screen", kClear},
};

void kStartConsoleShell(void) {
  char command_buffer[0x200];
  int command_buffer_idx = 0;

  printf(CONSOLE_SHELL_PROMPT_MESSAGE);

  while (1) {
    uint8 key = getch();

    if (key == (uint8)kBacksapce) {
      int x, y;
      kGetCursor(&x, &y);
      kPrintStringXY(x - 1, y, " ");
      kSetCursor(x - 1, y);

      --command_buffer_idx;
    } else if (key == (uint8)kEnter) {
      printf("\n");

      if (command_buffer_idx > 0) {
        command_buffer[command_buffer_idx] = 0;
        kExecuteCommand(command_buffer);
      }

      printf(CONSOLE_SHELL_PROMPT_MESSAGE);
      memset(command_buffer, 0, 0x200);
      command_buffer_idx = 0;
    } else if (key == (uint8)kLshift || key == (uint8)kRshift ||
               key == (uint8)kCapslock || key == (uint8)kNumlock ||
               key == (uint8)kScrolllock) {
      ;
    } else {
      if (key == (uint8)kTab) {
        key = ' ';
      }

      if (command_buffer_idx < 0x200) {
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