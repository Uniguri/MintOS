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
#include "Synchronization.h"
#include "Task.h"
#include "Tick.h"

ShellCommandEntry command_table[] = {
    {"help", "Show Help", kConsoleHelp},
    {"clear", "Clear Screen", kConsoleClear},
    {"totalram", "Show Total RAM Size", kConsoleShowRamSize},
    {"settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)",
     kConsoleSetTimer},
    {"wait", "Wait ms Using PIT, ex)wait 100(ms)", kConsoleWaitUsingPIT},
    {"rdtsc", "Read Time Stamp Counter", kConsolePrintTimeStampCounter},
    {"date", "Show Date", kConsoleShowDate},
    {"time", "Show Time", kConsoleShowTime},
    {"createtask", "Create Task", kConsoleCreateTestTask},
    {"changepriority",
     "Change Task Priority, ex)changepriority 1(ID) 2(Priority)",
     kConsoleChangeTaskPriority},
    {"ps", "Show Task List. ex) ps {do only show alive; 1 or 0}",
     kConsoleShowTaskList},
    {"taskinfo", "Print information of task, ex) taskinfo 1", kConsoleTaskInfo},
    {"kill", "End Task, ex)kill 1(ID) {do kill only alive; 1 or 0}",
     kConsoleKillTask},
    {"cpuload", "Show Processor Load", kConsoleCPULoad},
    {"testmutex", "Test Mutex Function", kConsoleTestMutex},
    {"testthread", "Test Thread And Process Function", kTestThread},
    {"testpie", "Test PIE Calculation", kTestPIE},
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
    ++arguments;
  } else {
    space_index = command_buffer_length;
    arguments = command_buffer + space_index + 1;
  }

  const size_t number_of_command =
      sizeof(command_table) / sizeof(ShellCommandEntry);
  for (size_t i = 0; i < number_of_command; ++i) {
    const char* command = command_table[i].command;
    if (!memcmp(command_buffer, command, space_index)) {
      return command_table[i].function(arguments);
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
  const char* space_buf = strchr(cur_buf, ' ');
  int64 space_idx_of_cur_buf = space_buf - param_buf;
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

static void kConsoleHelp(const char* parameter_buffer) {
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
static void kConsoleClear(const char* parameter_buffer) {
  kClearScreen();
  kSetCursor(0, 1);
}

static void kConsoleShowRamSize(const char* parameter_buffer) {
  printf("Total RAM size = %d MB.\n", kGetRamSize());
}

static void kConsoleSetTimer(const char* parameter_buffer) {
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

static void kConsoleWaitUsingPIT(const char* parameter_buffer) {
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

static void kConsolePrintTimeStampCounter(const char* parameter_buffer) {
  printf("Time Stamp Counter = %q\n", kReadTSC());
}

static void kConsoleShowDate(const char* parameter_buffer) {
  uint8 sec, min, hour;
  uint8 day_of_week, day_of_month, month;
  uint16 year;

  kReadRTCTime(&hour, &min, &sec);
  kReadRTCDate(&year, &month, &day_of_month, &day_of_week);

  printf("%d/%d/%d %s\n", year, month, day_of_month,
         kConvertDayOfWeekToString(day_of_week));
}

static void kConsoleShowTime(const char* parameter_buffer) {
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
  uint32 margin = (running_task->id_link.id & 0xFFFFFFFF) % 10;
  int i = 0, x = 0, y = 0;
  for (int _ = 0; _ < 2000; ++_) {
    switch (i) {
      case 0:
        if (++x >= (CONSOLE_WIDTH - margin)) {
          i = 1;
        }
        break;
      case 1:
        if (++y >= (CONSOLE_HEIGHT - margin)) {
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

    // kSchedule();
  }

  kExitTask();
}

void kTestTask2(void) {
  Character* screen = (Character*)CONSOLE_VIDEO_MEMORY_ADDRESS;
  TaskControlBlock* running_task = kGetRunningTask();
  const char data[4] = {'-', '\\', '|', '/'};

  uint32 offset = (running_task->id_link.id & 0xFFFFFFFF) * 2;
  offset = CONSOLE_WIDTH * CONSOLE_HEIGHT -
           (offset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

  uint32 i = 0;
  while (1) {
    screen[offset].charactor = data[i % 4];
    screen[offset].attribute = (offset % 15) + 1;
    ++i;

    // kSchedule();
  }
}

static void kConsoleCreateTestTask(const char* parameter_buffer) {
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
        if (!kCreateTask(kTaskPriorityLow | TASK_FLAG_THREAD, 0, 0,
                         (uint64)kTestTask1)) {
          break;
        }
      }
      break;
    }
    case 2: {
      int i;
      for (i = 0; i < count; ++i) {
        if (!kCreateTask(kTaskPriorityLow | TASK_FLAG_THREAD, 0, 0,
                         (uint64)kTestTask2)) {
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

static void kConsoleChangeTaskPriority(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  if (!GET_NEXT_PARAM()) {
    printf("Usage: changepriority [task id] [priority; 0~4]\n");
    return;
  }
  uint64 task_id;
  if (!memcmp(param, "0x", 2)) {
    task_id = Uint64FromHexString(param);
  } else {
    task_id = Uint64FromDecimalString(param);
  }

  if (!GET_NEXT_PARAM()) {
    printf("Usage: changepriority [task id] [priority; 0~4]\n");
    return;
  }
  uint64 priority = Uint64FromDecimalString(param);

  kChangeTaskPriority(task_id, priority);
}

static void kConsoleShowTaskList(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  printf("=========== Task Total Count [%d] ===========\n", kGetTaskCount());
  bool do_only_print_present = 1;
  if (GET_NEXT_PARAM()) {
    do_only_print_present = (bool)Int32FromDecimalString(param);
  }

  int count = 0;
  for (int i = 0; i < TASK_MAX_COUNT; ++i) {
    TaskControlBlock* tcb = kGetTCBInTCBPool(i);

    if (do_only_print_present) {
      if (IS_TASK_PRESENT(tcb) && !IS_TASK_END_TASK(tcb)) {
        printf("[%d] Task ID[%p], Priority[%d], Flags[%p], Thread[%d]\n",
               1 + count++, GET_TCB_OFFSET_FROM_ID(tcb->id_link.id),
               GET_TASK_PRIORITY(tcb), tcb->flags,
               kGetListCount(&tcb->child_thread_list));
        printf("    Parent PID[%p], Memory Address[%p], Size[%p]\n",
               tcb->parent_process_id, tcb->memory_addr, tcb->memory_size);
      }
    } else {
      printf("[%d] Task ID[%p], Priority[%d], Flags[%p], Thread[%d]\n",
             1 + count++, tcb->id_link.id, GET_TASK_PRIORITY(tcb), tcb->flags,
             kGetListCount(&tcb->child_thread_list));
      printf("    Parent PID[%p], Memory Address[%p], Size[%p]\n",
             tcb->parent_process_id, tcb->memory_addr, tcb->memory_size);
    }

    if (count % 10 == 0) {
      printf("Press any key to continue... ('q' is exit) : ");
      if (getch() == 'q') {
        printf("\n");
        break;
      } else {
        printf("\n");
      }
    }
  }
}

static void kConsoleTaskInfo(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);
  if (!GET_NEXT_PARAM()) {
    printf("Usage: taskinfo [task id]\n");
    return;
  }

  uint64 task_id;
  if (!memcmp(param, "0x", 2)) {
    task_id = Int64FromDecimalString(param);
  } else {
    task_id = Int64FromHexString(param);
  }

  TaskControlBlock* tcb = kGetTCBInTCBPool(GET_TCB_OFFSET_FROM_ID(task_id));
  printf(
      "Task [%p] : Address = %p\n"
      "    flags = %p\n"
      "    context = {\n"
      "        GS: %p, FS: %p, ES: %p, DS: %p, CS: %p, SS: %p\n"
      "    }\n"
      "    stack_addr = %p\n",
      tcb->id_link.id, tcb, tcb->flags, tcb->context.gs, tcb->context.fs,
      tcb->context.es, tcb->context.ds, tcb->context.cs, tcb->context.ss,
      tcb->stack_addr);
}

// Maybe in this function, something wrong.
// After "createtask 2 1022" and "kill all", the idle task's
// TCB(0x800000+0xE8) is corrupted. After that operations, its ES and DS, SS
// is 0x8000??.
static void kConsoleKillTask(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  if (!GET_NEXT_PARAM()) {
    printf("Usage: kill [task id or all] {do kill only alive; 1 or 0}\n");
    return;
  }

  uint64 task_id;
  if (!memcmp(param, "all", 3)) {
    for (int i = 0; i < TASK_MAX_COUNT; ++i) {
      TaskControlBlock* tcb = kGetTCBInTCBPool(i);
      const uint64 id = tcb->id_link.id;
      if (IS_TASK_PRESENT(tcb) && !IS_TASK_SYSTEM_TASK(tcb)) {
        kEndTask(id);
      }
    }
    return;
  } else if (!memcmp(param, "0x", 2)) {
    task_id = Uint64FromHexString(param);
  } else {
    task_id = Uint64FromDecimalString(param);
  }
  if (GET_TCB_OFFSET_FROM_ID(task_id) >= TASK_MAX_COUNT) {
    printf("Invalid task id\n");
    return;
  }

  bool do_kill_only_alive = 1;
  if (GET_NEXT_PARAM()) {
    do_kill_only_alive = (bool)Int32FromDecimalString(param);
  }

  if (do_kill_only_alive) {
    if (kIsTaskExist(task_id)) {
      kEndTask(TASK_ID_PRESENT | task_id);
    }
  } else {
    kEndTask(task_id);
  }
}

static void kConsoleCPULoad(const char* parameter_buffer) {
  printf("Processor Load : %d%%\n", kGetProcessorLoad());
}

static Mutex mutex;
static volatile uint64 adder;
void kConsolePrintNumberTask(void) {
  uint64 tick_count = kGetTickCount();
  while (kGetTickCount() - tick_count < 50) {
    kSchedule();
  }
  for (int i = 0; i < 5; ++i) {
    kLockMutex(&mutex);
    printf("Task ID [%p] Value[%d]\n", kGetRunningTask()->id_link.id, adder++);
    kUnlockMutex(&mutex);
    for (int j = 0; j < 30000; ++j)
      ;
  }
  tick_count = kGetTickCount();
  while (kGetTickCount() - tick_count < 1000) {
    kSchedule();
  }
  kExitTask();
}

static void kConsoleTestMutex(const char* parameter_buffer) {
  adder = 1;
  kInitializeMutex(&mutex);
  for (int i = 0; i < 3; ++i) {
    kCreateTask(kTaskPriorityLow | TASK_FLAG_THREAD, 0, 0,
                (uint64)kConsolePrintNumberTask);
  }
  printf("Wit Until %d Task End\n", 3);
  getch();
}

static void kCreateThreadTask(void) {
  for (int i = 0; i < 3; ++i) {
    kCreateTask(kTaskPriorityLow | TASK_FLAG_THREAD, 0, 0, (uint64)kTestTask2);
  }
  while (1) {
    kSleep(1);
  }
}

static void kTestThread(const char* parameter_buffer) {
  TaskControlBlock* process =
      kCreateTask(kTaskPriorityLow | TASK_FLAG_PROCESS, (void*)0xEEEEEEEE,
                  0x1000, (uint64)kCreateThreadTask);
}

static volatile uint64 random_value = 0;

uint64 kRandom(void) {
  random_value = (random_value * 412153 + 5571031) >> 16;
  return random_value;
}

static void kFPUTestTask(void) {
  const char data[4] = {'-', '\\', '|', '/'};
  TaskControlBlock* running_task = kGetRunningTask();
  Character* screen = (Character*)CONSOLE_VIDEO_MEMORY_ADDRESS;
  int offset = (running_task->id_link.id & 0xFFFFFFFF) * 2;
  offset = CONSOLE_WIDTH * CONSOLE_HEIGHT -
           (offset % (CONSOLE_BACKGROUND_WHITE * CONSOLE_HEIGHT));

  uint64 count = 0;
  while (1) {
    double value1 = 1;
    double value2 = 1;
    for (int i = 0; i < 10; ++i) {
      uint64 rand_val = kRandom();
      value1 *= (double)rand_val;
      value2 *= (double)rand_val;

      kSleep(1);

      rand_val = kRandom();
      value1 /= (double)rand_val;
      value2 /= (double)rand_val;
    }

    if (value1 != value2) {
      printf("Value is not same [%f] != [%f]\n", value1, value2);
      break;
    }
    ++count;

    screen[offset].charactor = data[count % 4];
    screen[offset].attribute = (offset % 15) + 1;
  }
}

static void kTestPIE(const char* parameter_buffer) {
  const double res = (double)355 / 113;
  printf("355 / 113 = %d.%d%d\n", (uint64)res, (uint64)(res * 10) % 10,
         (uint64)(res * 100) % 10);
  for (int i = 0; i < 100; ++i) {
    kCreateTask(kTaskPriorityLow | TASK_FLAG_THREAD, 0, 0,
                (uint64)kFPUTestTask);
  }
}

#undef MAKE_LIST_AND_PARAM
#undef GET_NEXT_PARAM