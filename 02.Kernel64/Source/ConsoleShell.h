#ifndef MINTOS_CONSOLESHELL_H_
#define MINTOS_CONSOLESHELL_H_

#include "Types.h"

#define CONSOLE_BUFFER_SIZE (0x200)
#define CONSOLE_SHELL_MAX_COMMAND_BUFFER_COUNT (300)
#define CONSOLE_SHELL_PROMPT_MESSAGE ("MINT64>")

typedef void (*CommandFunction)(const char* parameter_buffer);

#pragma pack(push, 1)
typedef struct kShellCommandEntryStruct {
  char* command;
  char* help;
  CommandFunction function;
} ShellCommandEntry;

typedef struct kParameterListStruct {
  const char* buffer;
  size_t length;
  size_t current_position;
} ParameterList;
#pragma pack(pop)

void kStartConsoleShell(void);
void kExecuteCommand(const char* command_buffer);
void kInitializeParameter(ParameterList* parameter_list, const char* parameter);
int kGetNextParameter(ParameterList* parameter_list, char* parameter);

void kHelp(const char* parameter_buffer);
void kClear(const char* parameter_buffer);
void kShowRamSize(const char* parameter_buffer);
void kSetTimer(const char* parameter_buffer);
void kWaitUsingPIT(const char* parameter_buffer);
void kPrintTimeStampCounter(const char* parameter_buffer);
void kShowDate(const char* parameter_buffer);
void kShowTime(const char* parameter_buffer);
void kCreateTestTask(const char* parameter_buffer);

#endif