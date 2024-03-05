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

static void kConsoleHelp(const char* parameter_buffer);
static void kConsoleClear(const char* parameter_buffer);
static void kConsoleShowRamSize(const char* parameter_buffer);
static void kConsoleSetTimer(const char* parameter_buffer);
static void kConsoleWaitUsingPIT(const char* parameter_buffer);
static void kConsolePrintTimeStampCounter(const char* parameter_buffer);
static void kConsoleShowDate(const char* parameter_buffer);
static void kConsoleShowTime(const char* parameter_buffer);
static void kConsoleCreateTestTask(const char* parameter_buffer);
static void kConsoleChangeTaskPriority(const char* parameter_buffer);
static void kConsoleShowTaskList(const char* parameter_buffer);
static void kConsoleKillTask(const char* parameter_buffer);
static void kConsoleCPULoad(const char* parameter_buffer);

#endif