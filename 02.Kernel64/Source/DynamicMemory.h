#ifndef MINTOS_DYNAMIC_MEMORY_H_
#define MINTOS_DYNAMIC_MEMORY_H_

#include "Macro.h"
#include "Types.h"

#define DYNAMIC_MEMORY_START_ADDRESS                                     \
  ((void*)((TASK_STACK_POOL_ADDRESS + TASK_STACK_SIZE * TASK_MAX_COUNT + \
            0xFFFFF) &                                                   \
           0xFFFFFFFFFFF00000))

#define DYNAMIC_MEMORY_MIN_SIZE (BYTE_FROM_KB(1))

#define DYNAMIC_MEMORY_EXIST (0x01)
#define DYNAMIC_MEMORY_EMPTY (0x00)

typedef struct kBitmapStruct {
  uint8* bitmap;
  uint64 exist_bit_count;
} Bitmap;

typedef struct kDynamicMemoryManagerStruct {
  uint32 max_level;
  uint32 smallest_block_count;
  size_t used_size;

  uint64 start_address;
  uint64 end_address;

  // list for indicating which block list allocated block is in.
  uint8* allocated_block_list_index;
  Bitmap* bitmap_of_level;
} DynamicMemory;

void kInitializeDynamicMemory(void);

void* kAllocateMemory(size_t size);

bool kFreeMemory(void* address);

static size_t kCalculateDynamicMemorySize(void);

static uint32 kCalculateMetaBlockCount(size_t dynamic_ram_size);

static int kAllocationBuddyBlock(size_t aligned_size);

static size_t kGetBuddyBlockSize(size_t size);

static int kGetBlockListIndexOfMatchSize(size_t aligned_size);

static int kFindFreeBlockInBitmap(int block_level);

static void kSetFlagInBitmap(int block_level, int offset, uint8 flag);

static bool kFreeBuddyBlock(int block_level, int block_offset);

static uint8 kGetFlagInBitmap(int block_level, int offset);

void kGetDynamicMemoryInformation(uint64* dynamic_memory_start_addr,
                                  size_t* dynamic_memory_total_size,
                                  size_t* meta_data_size,
                                  size_t* used_memory_size);

DynamicMemory* kGetDynamicMemoryManager(void);

#endif