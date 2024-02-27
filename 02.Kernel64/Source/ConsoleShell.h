#ifndef MINTOS_CONSOLESHELL_H_
#define MINTOS_CONSOLESHELL_H_

#define CONSOLE_SHELL_MAX_COMMAND_BUFFER_COUNT (300)
#define CONSOLE_SHELL_PROMPT_MESSAGE ("MINT64>")

typedef void (*CommandFunction)(const char* parameter_buffer);

#pragma pack(push, 1)
typedef struct kShellCommandEntryStruct {
  char* command;
  char* help;
  CommandFunction function;
} ShellCommandEntry;
#pragma pack(pop)

void kStartConsoleShell(void);
void kExecuteCommand(const char* command_buffer);

void kHelp(const char* parameter_buffer);
void kClear(const char* parameter_buffer);

#endif