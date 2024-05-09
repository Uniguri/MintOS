#include "ConsoleShell.h"

#include "Console.h"
#include "DynamicMemory.h"
#include "FileSystem.h"
#include "HardDisk.h"
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
    {"testthread", "Test Thread And Process Function", kConsoleTestThread},
    {"testrandom", "Test random", kConsoleTestRandom},
    {"testpie", "Test PIE Calculation", kConsoleTestPIE},
    {"dynamicmeminfo", "Show Dyanmic Memory Information",
     kConsoleShowDyanmicMemoryInformation},
    {"testseqalloc", "Test Sequential Allocation & Free",
     kConsoleTestSequentialAllocation},
    {"testranalloc", "Test Random Allocation & Free",
     kConsoleTestRandomAllocation},
    {"hddinfo", "Show HDD Information", kShowHDDInformation},
    {"readsector", "Read HDD Sector, ex)readsector 0(LBA) 10(count)",
     kReadSector},
    {"writesector", "Write HDD Sector, ex)writesector 0(LBA) 10(count)",
     kWriteSector},
    {"mounthdd", "Mount HDD", kMountHDD},
    {"formathdd", "Format HDD", kFormatHDD},
    {"filesysteminfo", "Show File System Information",
     kShowFileSystemInformation},
    {"createfile", "Create File, ex)createfile a.txt",
     kCreateFileInRootDirectory},
    {"deletefile", "Delete File, ex)deletefile a.txt",
     kDeleteFileInRootDirectory},
    {"ls", "Show Directory", kShowRootDirectory},
    {"writefile", "Write Data To File, ex) writefile a.txt", kWriteDataToFile},
    {"readfile", "Read Data From File, ex) readfile a.txt", kReadDataFromFile},
    {"testfileio", "Test File I/O Function", kTestFileIO},
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

    if (i != 0 && i % 20 == 0) {
      printf("Press any key to continue... ('q' is exit) : ");
      if (getch() == 'q') {
        printf("\n");
        break;
      }
      printf("\n");
    }
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
    for (int j = 0; j < 30000; ++j);
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
  printf("Wait Until %d Task End\n", 3);
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

static void kConsoleTestThread(const char* parameter_buffer) {
  TaskControlBlock* process =
      kCreateTask(kTaskPriorityLow | TASK_FLAG_PROCESS, (void*)0xEEEEEEEE,
                  0x1000, (uint64)kCreateThreadTask);
}

static volatile uint64 random_value = 0;

uint64 kRandom(void) {
  random_value = (random_value * 1103515245 + 12345) % 2147483648;
  return random_value;
}

static void kConsoleTestRandom(const char* parameter_buffer) {
  for (int i = 0; i < 100; ++i) {
    printf("%d, ", kRandom());
  }
  printf("\n");
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

static void kConsoleTestPIE(const char* parameter_buffer) {
  const double res = (double)355 / 113;
  printf("355 / 113 = %d.%d%d\n", (uint64)res, (uint64)(res * 10) % 10,
         (uint64)(res * 100) % 10);
  for (int i = 0; i < 100; ++i) {
    kCreateTask(kTaskPriorityLow | TASK_FLAG_THREAD, 0, 0,
                (uint64)kFPUTestTask);
  }
}

static void kConsoleShowDyanmicMemoryInformation(const char* parameter_buffer) {
  uint64 start_addr;
  size_t total_size, meta_data_size, used_memory_size;
  kGetDynamicMemoryInformation(&start_addr, &total_size, &meta_data_size,
                               &used_memory_size);

  printf("============ Dynamic Memory Information ============\n");
  printf("Start Address: [%p]\n", start_addr);
  printf("Total Size: [%p] bytes, [%d] MB\n", total_size,
         MB_FROM_BYTE(total_size));
  printf("Meta Size: [%p] bytes, [%d] KB\n", meta_data_size,
         KB_FROM_BYTE(meta_data_size));
  printf("Used Size: [%p] bytes, [%d] KB\n", used_memory_size,
         KB_FROM_BYTE(used_memory_size));
}

static void kConsoleTestSequentialAllocation(const char* parameter_buffer) {
  DynamicMemory* mem = kGetDynamicMemoryManager();
  for (int i = 0; i < mem->max_level; ++i) {
    for (int j = 0; j < (mem->smallest_block_count >> i); ++j) {
      uint64* buffer = kAllocateMemory(DYNAMIC_MEMORY_MIN_SIZE << i);
      if (!buffer) {
        printf("\nAllocation Fail\n");
        return;
      }

      for (int k = 0; k < (DYNAMIC_MEMORY_MIN_SIZE << i) / 8; ++k) {
        buffer[k] = k;
      }
      for (int k = 0; k < (DYNAMIC_MEMORY_MIN_SIZE << i) / 8; ++k) {
        if (buffer[k] != k) {
          printf("\nCompare Fail\n");
          return;
        }
      }

      printf(".");
    }

    for (int j = 0; j < (mem->smallest_block_count >> i); ++j) {
      if (kFreeMemory((void*)(mem->start_address +
                              (DYNAMIC_MEMORY_MIN_SIZE << i) * j)) == false) {
        printf("\nFree Fail\n");
        return;
      }
      printf(".");
    }
    printf("\n");
  }
}

static void kConsoleRandomAllocationTask(void) {
  TaskControlBlock* task = kGetRunningTask();
  const int y = (task->id_link.id) % 15 + 9;
  for (int i = 0; i < 10; ++i) {
    char buffer[200];
    size_t mem_size;
    uint8* alloc_buf;
    do {
      mem_size = ((kRandom() % (32 * 1024)) + 1) * 1024;
      alloc_buf = kAllocateMemory(mem_size);

      if (!alloc_buf) {
        kSleep(1);
      }
    } while (!alloc_buf);
    snprintf(buffer, 200, "|Address: [%p] Size: [%p] Allocation Success      ",
             alloc_buf, mem_size);
    kPrintStringXY(20, y, buffer);
    kSleep(200);

    snprintf(buffer, 200, "|Address: [%p] Size: [%p] Data Write...           ",
             alloc_buf, mem_size);
    kPrintStringXY(20, y, buffer);
    for (int j = 0; j < mem_size / 2; ++j) {
      alloc_buf[j] = kRandom() & 0xFF;
      alloc_buf[j + mem_size / 2] = alloc_buf[j];
    }
    kSleep(200);

    snprintf(buffer, 200, "|Address: [%p] Size: [%p] Data Verify...          ",
             alloc_buf, mem_size);
    kPrintStringXY(20, y, buffer);
    for (int j = 0; j < mem_size / 2; ++j) {
      if (alloc_buf[j] != alloc_buf[j + mem_size / 2]) {
        printf("Task ID[%p] Verify Fail\n", task->id_link.id);
        kExitTask();
      }
    }

    kFreeMemory(alloc_buf);
    kSleep(200);
  }

  kExitTask();
}

static void kConsoleTestRandomAllocation(const char* parameter_buffer) {
  for (int i = 0; i < 1000; ++i) {
    kCreateTask(kTaskPriorityLowest | TASK_FLAG_THREAD, 0, 0,
                (uint64)kConsoleRandomAllocationTask);
  }
}

static void kShowHDDInformation(const char* parameter_buffer) {
  HDDInformation hdd;
  if (!kReadHDDInformation(true, true, &hdd)) {
    printf("HDD Information Read Fail\n");
    return;
  }

  char buffer[0x200];
  printf("============ Primary Master HDD Information ============\n");
  memcpy(buffer, hdd.model_number, sizeof(hdd.model_number));
  buffer[sizeof(hdd.model_number) - 1] = 0;
  printf("Model Number:\t %s\n", buffer);
  memcpy(buffer, hdd.serial_number, sizeof(hdd.serial_number));
  buffer[sizeof(hdd.serial_number) - 1] = 0;
  printf("Model Number:\t %s\n", buffer);
  printf("Head Count:\t %d\n", hdd.number_of_head);
  printf("Cylinder Count:\t %d\n", hdd.number_of_cylinder);
  printf("Sector Count:\t %d\n", hdd.number_of_sector_per_cylinder);
  printf("Total Sector:\t %d Sector, %d MB\n", hdd.total_sectors,
         hdd.total_sectors / 2 / 1024);
}

static void kReadSector(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  if (!GET_NEXT_PARAM()) {
    goto ERROR;
  }
  const uint32 lba = Int32FromDecimalString(param);

  if (!GET_NEXT_PARAM()) {
    goto ERROR;
  }
  const int sector_count = Int32FromDecimalString(param);

  char* buffer = kAllocateMemory(sector_count * 512);
  if (kReadHDDSector(true, true, lba, sector_count, buffer) == sector_count) {
    printf("LBA [%d], [%d] Sector Read Success", lba, sector_count);
    bool do_exit = false;
    for (int i = 0; i < sector_count; ++i) {
      for (int j = 0; j < 512; ++j) {
        if (!(j == 0 && i == 0) && j % 256 == 0) {
          printf("\nPress any key to continue... ('q' is exit): ");
          if (getch() == 'q') {
            do_exit = true;
            break;
          }
        }
        if (j % 16 == 0) {
          printf("\n[LBA:%d, Offset:%d]\t ", lba + i, j);
        }

        const int data = buffer[i * 512 + j] & 0xFF;
        if (data < 16) {
          printf("0");
        }
        printf("%x ", data);
      }

      if (do_exit) {
        break;
      }
    }
    printf("\n");
  } else {
    printf("Read Fail\n");
  }
  kFreeMemory(buffer);
  return;
ERROR:
  printf("ex) readsector 0(LBA) 10(count)\n");
  return;
}

static void kWriteSector(const char* parameter_buffer) {
  static uint32 write_count = 0;
  MAKE_LIST_AND_PARAM(parameter_buffer);

  if (!GET_NEXT_PARAM()) {
    goto ERROR;
  }
  const uint32 lba = Int32FromDecimalString(param);

  if (!GET_NEXT_PARAM()) {
    goto ERROR;
  }
  const int sector_count = Int32FromDecimalString(param);

  ++write_count;
  char* buffer = kAllocateMemory(sector_count * 512);
  for (int i = 0; i < sector_count; ++i) {
    for (int j = 0; j < 512; j += 8) {
      *(uint32*)&buffer[i * 512 + j] = lba + i;
      *(uint32*)&buffer[i * 512 + j + 4] = write_count;
    }
  }

  if (kWriteHDDSector(true, true, lba, sector_count, buffer) != sector_count) {
    printf("Write fail\n");
    kFreeMemory(buffer);
    return;
  }
  printf("LBA [%d], [%d] Sector Write Success", lba, sector_count);

  bool do_exit = false;
  for (int i = 0; i < sector_count; ++i) {
    for (int j = 0; j < 512; ++j) {
      if (!(j == 0 && i == 0) && j % 256 == 0) {
        printf("\nPress any key to continue... ('q' is exit): ");
        if (getch() == 'q') {
          do_exit = true;
          break;
        }
      }
      if (j % 16 == 0) {
        printf("\n[LBA:%d, Offset:%d]\t ", lba + i, j);
      }

      const int data = buffer[i * 512 + j] & 0xFF;
      if (data < 16) {
        printf("0");
      }
      printf("%x ", data);
    }

    if (do_exit) {
      break;
    }
  }
  printf("\n");

  kFreeMemory(buffer);
  return;
ERROR:
  printf("ex) writesector 0(LBA) 10(count)\n");
  return;
}

static void kMountHDD(const char* parameter_buffer) {
  if (kMount() == false) {
    printf("HDD Mount Fail\n");
  }
}

static void kFormatHDD(const char* parameter_buffer) {
  if (kFormat() == false) {
    printf("HDD Format Fail\n");
  }
}

static void kShowFileSystemInformation(const char* parameter_buffer) {
  FileSystemManager manager;
  kGetFileSystemInformation(&manager);

  printf("================== File System Information ==================\n");
  printf("Mouted:\t\t\t\t\t %d\n", manager.mounted);
  printf("Reserved Sector Count:\t\t\t %d Sector\n",
         manager.reserved_sector_count);
  printf("Cluster Link Table Start Address:\t %d Sector\n",
         manager.cluster_link_area_start_address);
  printf("Cluster Link Table Size:\t\t %d Sector\n",
         manager.cluster_link_area_size);
  printf("Data Area Start Address:\t\t %d Sector\n",
         manager.data_area_start_address);
  printf("Total Cluster Count:\t\t\t %d Cluster\n",
         manager.total_cluster_count);
}

static void kCreateFileInRootDirectory(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  DirectoryEntry entry;
  size_t file_name_len = GET_NEXT_PARAM();

  if (file_name_len == 0) {
    goto ERROR;
  } else if (file_name_len > sizeof(entry.file_name) - 1) {
    printf("Too long file name\n");
    return;
  }

  FILE* file = fopen(param, "w");
  if (!file) {
    printf("File Create Fail\n");
    return;
  }
  fclose(file);
  return;

ERROR:
  printf("ex) createfile a.txt");
  return;
}

static void kDeleteFileInRootDirectory(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);

  DirectoryEntry entry;
  size_t file_name_len = GET_NEXT_PARAM();

  if (file_name_len == 0) {
    goto ERROR;
  } else if (file_name_len > sizeof(entry.file_name) - 1 ||
             file_name_len == 0) {
    printf("Too Long File Name\n");
    return;
  }

  if (remove(param) != 0) {
    printf("File Not Found or File Opened\n");
    return;
  }
  return;

ERROR:
  printf("ex) deletefile a.txt");
  return;
}

static void kShowRootDirectory(const char* parameter_buffer) {
  char buffer[400];
  char temp_value[50];
  FileSystemManager manager;

  kGetFileSystemInformation(&manager);

  DIR* directory = opendir("/");
  if (!directory) {
    printf("Root Directory Open Fail\n");
    return;
  }

  int total_count = 0;
  uint32 total_byte = 0;
  uint32 used_cluster_count = 0;
  while (1) {
    struct dirent* entry = readdir(directory);
    if (!entry) {
      break;
    }
    ++total_count;
    total_byte += entry->file_size;

    if (entry->file_size == 0) {
      ++used_cluster_count;
    } else {
      used_cluster_count += (entry->file_size + FILESYSTEM_CLUSTER_SIZE - 1) /
                            FILESYSTEM_CLUSTER_SIZE;
    }
  }

  rewinddir(directory);
  int count = 0;
  while (1) {
    struct dirent* entry = readdir(directory);
    if (!entry) {
      break;
    }

    memset(buffer, ' ', sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = 0;

    memcpy(buffer, entry->d_name, strlen(entry->d_name));
    snprintf(temp_value, sizeof(temp_value), "%d Byte", entry->file_size);
    memcpy(buffer + 30, temp_value, strlen(temp_value));

    snprintf(temp_value, sizeof(temp_value), "%p Cluster",
             entry->start_cluster_index);
    memcpy(buffer + 50, temp_value, strlen(temp_value) + 1);
    printf("    %s\n", buffer);

    if (count != 0 && count % 20 == 0) {
      printf("Press any key to continue... ('q' is exit) : ");
      if (getch() == 'q') {
        printf("\n");
        break;
      }
    }
    ++count;
  }

  printf("\t\tTotal File Count: %d\n", total_count);
  printf("\t\tTotal File Size: %d KByte (%d Cluster)\n", total_byte,
         used_cluster_count);

  printf("\t\tFree Space: %d KByte (%d Cluster)\n",
         (manager.total_cluster_count - used_cluster_count) *
             FILESYSTEM_CLUSTER_SIZE / 1024,
         manager.total_cluster_count - used_cluster_count);

  closedir(directory);
}

static void kWriteDataToFile(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);
  int length = GET_NEXT_PARAM();
  if (length == 0 || length > FILESYSTEM_MAX_FILE_NAME_LENGTH - 1) {
    printf("Too Long File Name\n");
    return;
  }

  FILE* fp = fopen(param, "w");
  if (!fp) {
    printf("%s File Open Fail\n", param);
    return;
  }

  int enter_count = 0;
  while (1) {
    uint8 key = getch();
    if (key == kEnter) {
      if (++enter_count >= 3) {
        break;
      }
    } else {
      enter_count = 0;
    }

    printf("%c", key);
    if (fwrite(&key, 1, 1, fp) != 1) {
      printf("File Write Fail\n");
      break;
    }
  }

  fclose(fp);

  return;
ERROR:
  printf("ex) writefile a.txt");
  return;
}

static void kReadDataFromFile(const char* parameter_buffer) {
  MAKE_LIST_AND_PARAM(parameter_buffer);
  int length = GET_NEXT_PARAM();
  if (length == 0 || length > FILESYSTEM_MAX_FILE_NAME_LENGTH - 1) {
    printf("Too Long File Name\n");
    return;
  }

  FILE* fp = fopen(param, "r");
  if (!fp) {
    printf("%s File Open Fail\n", param);
    return;
  }

  int enter_count = 0;
  while (1) {
    uint8 key;
    if (fread(&key, 1, 1, fp) != 1) {
      break;
    }
    printf("%c", key);
    if (key == kEnter) {
      ++enter_count;
      if (enter_count != 0 && enter_count % 20 == 0) {
        printf("Press any key to continue... ('q' is exit) : ");
        if (getch() == 'q') {
          printf("\n");
          break;
        }
        printf("\n");
        enter_count = 0;
      }
    }
  }

  fclose(fp);

  return;
ERROR:
  printf("ex) readfile a.txt");
  return;
}

static void kTestFileIO(const char* parameter_buffer) {
  printf("================== File I/O Function Test ==================\n");
  const uint32 max_file_size = 4 * 1024 * 1024;
  uint8* const buffer = kAllocateMemory(max_file_size);
  if (!buffer) {
    printf("Memory Allocation Fail\n");
    return;
  }
  remove("testfileio.bin");

  {
    printf("1. File Open Fail Test...");
    FILE* file = fopen("testfileio.bin", "r");
    if (file == nullptr) {
      printf("[Pass]\n");
    } else {
      printf("[Fail]\n");
      fclose(file);
    }
  }

  printf("2. File Create Test...");
  FILE* file = fopen("testfileio.bin", "w");
  if (file) {
    printf("[Pass]\n");
    printf("    File Handle [%p]\n", file);
  } else {
    printf("[Fail]\n");
  }

  {
    printf("3. Sequential Write Test(Cluster Size)...");
    int i;
    for (i = 0; i < 100; ++i) {
      memset(buffer, i, FILESYSTEM_CLUSTER_SIZE);
      if (fwrite(buffer, 1, FILESYSTEM_CLUSTER_SIZE, file) !=
          FILESYSTEM_CLUSTER_SIZE) {
        printf("[Fail]\n");
        printf("    %d Cluster Error\n", i);
        break;
      }
    }
    if (i >= 100) {
      printf("[Pass]\n");
    }
  }

  {
    printf("4. Sequential Read And Verify Test(Cluster Size)...");
    fseek(file, -100 * FILESYSTEM_CLUSTER_SIZE, SEEK_END);
    int i;
    for (i = 0; i < 100; ++i) {
      if (fread(buffer, 1, FILESYSTEM_CLUSTER_SIZE, file) !=
          FILESYSTEM_CLUSTER_SIZE) {
        printf("[Fail]\n");
        return;
      }
      for (int j = 0; j < FILESYSTEM_CLUSTER_SIZE; ++j) {
        if (buffer[j] != (uint8)i) {
          printf("[Fail]\n");
          printf("    %d Cluster Error. [%X] != [%X]\n", i, buffer[j],
                 (uint8)i);
          break;
        }
      }
    }

    if (i >= 100) {
      printf("[Pass]\n");
    }
  }

  char temp_buffer[1024];

  {
    printf("5. Random Write Test...\n");
    memset(buffer, 0, max_file_size);
    fseek(file, -100 * FILESYSTEM_CLUSTER_SIZE, SEEK_CUR);
    fread(buffer, 1, max_file_size, file);
    int i;
    for (i = 0; i < 100; ++i) {
      const uint32 byte_count = kRandom() % (sizeof(temp_buffer) - 1) + 1;
      const uint32 random_offset = kRandom() % (max_file_size - byte_count);
      printf("    [%d] Offset [%d] Byte [%d]...", i, random_offset, byte_count);

      fseek(file, random_offset, SEEK_SET);
      memset(temp_buffer, i, byte_count);

      if (fwrite(temp_buffer, 1, byte_count, file) != byte_count) {
        printf("[Fail]\n");
        break;
      } else {
        printf("[Pass]\n");
      }

      memset(buffer + random_offset, i, byte_count);
    }

    fseek(file, max_file_size - 1, SEEK_SET);
    fwrite(&i, 1, 1, file);
    buffer[max_file_size - 1] = (uint8)i;
  }

  {
    printf("6. Random Read And Verify Test...\n");
    int i;
    for (i = 0; i < 100; ++i) {
      const uint32 byte_count = kRandom() % (sizeof(temp_buffer) - 1) + 1;
      const uint32 random_offset = kRandom() % (max_file_size - byte_count);
      printf("    [%d] Offset [%d] Byte [%d]...", i, random_offset, byte_count);

      fseek(file, random_offset, SEEK_SET);

      if (fread(temp_buffer, 1, byte_count, file) != byte_count) {
        printf("[Fail]\n");
        printf("    Read Fail\n");
        break;
      }

      if (memcmp(buffer + random_offset, temp_buffer, byte_count) != 0) {
        printf("[Fail]\n");
        printf("    Compare Fail\n");
        break;
      }

      printf("[Pass]\n");
    }
  }

  {
    printf("7. Sequential Write, Read And Verify Test(1024 Byte)...\n");
    fseek(file, -max_file_size, SEEK_CUR);
    for (int i = 0; i < (2 * 1024 * 1024 / 1024); ++i) {
      printf("    [%d] Offset [%d] Byte [%d] Write...", i, i * 1024, 1024);

      if (fwrite(buffer + i * 1024, 1, 1024, file) != 1024) {
        printf("[Fail]\n");
        break;
      } else {
        printf("[Pass]\n");
      }
    }

    fseek(file, -max_file_size, SEEK_SET);

    for (int i = 0; i < max_file_size / 1024; ++i) {
      printf("    [%d] Offset [%d] Byte [%d] Read And Verify...", i, i * 1024,
             1024);

      if (fread(temp_buffer, 1, 1024, file) != 1024) {
        printf("[Fail]\n");
        break;
      }
      if (memcmp(buffer + i * 1024, temp_buffer, 1024) != 0) {
        printf("[Fail]\n");
        break;
      } else {
        printf("[Pass]\n");
      }
    }
  }

  {
    printf("8. File Delete Fail Test...");
    if (remove("testfileio.bin") != 0) {
      printf("[Pass]\n");
    } else {
      printf("[Fail]\n");
    }
  }

  {
    printf("9. File Close Test...");
    if (fclose(file) == 0) {
      printf("[Pass]\n");
    } else {
      printf("[Fail]\n");
    }
  }

  {
    printf("10. File Delete Test...");
    if (remove("testfileio.bin") == 0) {
      printf("[Pass]\n");
    } else {
      printf("[Fail]\n");
    }
  }

  kFreeMemory(buffer);
}

#undef MAKE_LIST_AND_PARAM
#undef GET_NEXT_PARAM