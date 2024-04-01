#include "DynamicMemory.h"

#include "Memory.h"
#include "Synchronization.h"
#include "Task.h"

static DynamicMemory dynamic_memory;

void kInitializeDynamicMemory(void) {
  const size_t dynamic_memory_size = kCalculateDynamicMemorySize();
  const uint64 meta_block_count = kCalculateMetaBlockCount(dynamic_memory_size);

  const uint64 smallest_block_count =
      dynamic_memory_size / DYNAMIC_MEMORY_MIN_SIZE - meta_block_count;
  dynamic_memory.smallest_block_count = smallest_block_count;

  int max_level = 0;
  while ((smallest_block_count >> max_level) > 0) {
    ++max_level;
  }
  dynamic_memory.max_level = max_level;

  dynamic_memory.allocated_block_list_index = DYNAMIC_MEMORY_START_ADDRESS;
  for (int i = 0; i < smallest_block_count; ++i) {
    dynamic_memory.allocated_block_list_index[i] = 0xFF;
  }
  dynamic_memory.bitmap_of_level =
      (Bitmap*)(DYNAMIC_MEMORY_START_ADDRESS +
                sizeof(uint8) * smallest_block_count);

  uint8* cur_bitmap_addr =
      (uint8*)dynamic_memory.bitmap_of_level + sizeof(Bitmap) * max_level;
  for (int i = 0; i < max_level; ++i) {
    dynamic_memory.bitmap_of_level[i].bitmap = cur_bitmap_addr;
    dynamic_memory.bitmap_of_level[i].exist_bit_count = 0;

    const uint64 block_count = smallest_block_count >> i;
    for (int j = 0; j < block_count / 8; ++j) {
      *cur_bitmap_addr++ = 0;
    }

    const uint64 remainder = block_count % 8;
    if (remainder) {
      *cur_bitmap_addr = 0;
      if (remainder % 2) {
        SET_BIT(*cur_bitmap_addr, remainder - 1);
        dynamic_memory.bitmap_of_level[i].exist_bit_count = 1;
      }
      ++cur_bitmap_addr;
    }
  }

  dynamic_memory.start_address = (uint64)DYNAMIC_MEMORY_START_ADDRESS +
                                 meta_block_count * DYNAMIC_MEMORY_MIN_SIZE;
  dynamic_memory.end_address =
      (uint64)DYNAMIC_MEMORY_START_ADDRESS + dynamic_memory_size;
  dynamic_memory.used_size = 0;
}

void* kAllocateMemory(size_t size) {
  const size_t aligned_size = kGetBuddyBlockSize(size);
  if (aligned_size == 0) {
    return nullptr;
  }

  // When we does not have sufficient memory.
  if (dynamic_memory.start_address + dynamic_memory.used_size + aligned_size >
      dynamic_memory.end_address) {
    return nullptr;
  }

  const int allocated_block_offset = kAllocationBuddyBlock(aligned_size);
  if (allocated_block_offset == -1) {
    return nullptr;
  }

  const int index_of_block_list = kGetBlockListIndexOfMatchSize(aligned_size);
  const int64 relative_addr = aligned_size * allocated_block_offset;
  const int64 allocated_memory_index = relative_addr / DYNAMIC_MEMORY_MIN_SIZE;
  dynamic_memory.allocated_block_list_index[allocated_memory_index] =
      (uint8)index_of_block_list;
  dynamic_memory.used_size += aligned_size;

  return (void*)(dynamic_memory.start_address + relative_addr);
}

bool kFreeMemory(void* address) {
  if (!address) {
    return false;
  }

  const uint64 relative_addr = (uint64)address - dynamic_memory.start_address;
  const int64 allocated_memory_index = relative_addr / DYNAMIC_MEMORY_MIN_SIZE;
  const uint8 block_level =
      dynamic_memory.allocated_block_list_index[allocated_memory_index];
  if (block_level == 0xFF) {
    return false;
  }

  dynamic_memory.allocated_block_list_index[allocated_memory_index] = 0xFF;
  const size_t block_size = DYNAMIC_MEMORY_MIN_SIZE << block_level;
  const int bitmap_offset = relative_addr / block_size;
  if (kFreeBuddyBlock(block_level, bitmap_offset) == true) {
    dynamic_memory.used_size -= block_size;
    return true;
  }
  return false;
}

static uint32 kCalculateMetaBlockCount(size_t dynamic_ram_size) {
  const uint64 smallest_block_count =
      dynamic_ram_size / DYNAMIC_MEMORY_MIN_SIZE;
  const size_t size_of_allocated_block_list_index =
      smallest_block_count * sizeof(uint8);

  size_t size_of_bitmap = 0;
  for (int i = 0; (smallest_block_count >> i) > 0; ++i) {
    // Add size of Bitmap structure
    size_of_bitmap += sizeof(Bitmap);
    // Add size of actual bitmap(Bitmap.bitmap)
    size_of_bitmap += ((smallest_block_count >> i) + 7) / 8;
  }

  return (size_of_allocated_block_list_index + size_of_bitmap +
          DYNAMIC_MEMORY_MIN_SIZE - 1) /
         DYNAMIC_MEMORY_MIN_SIZE;
}

inline static size_t kCalculateDynamicMemorySize(void) {
  size_t ram_size = BYTE_FROM_MB(kGetRamSize());
  if (ram_size > BYTE_FROM_GB(3)) {
    ram_size = BYTE_FROM_GB(3);
  }
  return ram_size - (size_t)DYNAMIC_MEMORY_START_ADDRESS;
}

static int kAllocationBuddyBlock(size_t aligned_size) {
  const int fit_block_level = kGetBlockListIndexOfMatchSize(aligned_size);
  if (fit_block_level == -1) {
    return -1;
  }

  int level = 0;
  int free_offset = -1;
  const bool prev_interrupt_flag = kLockForSystemData();
  for (level = fit_block_level; level < dynamic_memory.max_level; ++level) {
    free_offset = kFindFreeBlockInBitmap(level);
    if (free_offset != -1) {
      break;
    }
  }
  if (free_offset == -1) {
    kUnlockForSystemData(prev_interrupt_flag);
    return -1;
  }

  kSetFlagInBitmap(level, free_offset, DYNAMIC_MEMORY_EMPTY);

  // When finded block's level > expected block's level, we split finded block's
  // level.
  if (level > fit_block_level) {
    for (int i = level - 1; i >= fit_block_level; --i) {
      kSetFlagInBitmap(i, free_offset * 2, DYNAMIC_MEMORY_EMPTY);
      kSetFlagInBitmap(i, free_offset * 2 + 1, DYNAMIC_MEMORY_EXIST);
      free_offset *= 2;
    }
  }
  kUnlockForSystemData(prev_interrupt_flag);
  return free_offset;
}

static size_t kGetBuddyBlockSize(size_t size) {
  for (int i = 0; i < dynamic_memory.max_level; ++i) {
    if (size <= (DYNAMIC_MEMORY_MIN_SIZE << i)) {
      return (DYNAMIC_MEMORY_MIN_SIZE << i);
    }
  }
  return 0;
}

static int kGetBlockListIndexOfMatchSize(size_t aligned_size) {
  for (int i = 0; i < dynamic_memory.max_level; ++i) {
    if (aligned_size <= (DYNAMIC_MEMORY_MIN_SIZE << i)) {
      return i;
    }
  }
  return -1;
}

static int kFindFreeBlockInBitmap(int block_level) {
  if (block_level > dynamic_memory.max_level ||
      dynamic_memory.bitmap_of_level[block_level].exist_bit_count == 0) {
    return -1;
  }

  const int max_block_count =
      dynamic_memory.smallest_block_count >> block_level;
  uint8* bitmap = dynamic_memory.bitmap_of_level[block_level].bitmap;
  for (int i = 0; i < max_block_count;) {
    if ((max_block_count - i) / 64 > 0) {
      const uint64 bitmap_8bytes_val = *(uint64*)&bitmap[i / 8];
      if (bitmap_8bytes_val == 0) {
        i += 64;
        continue;
      }
    }

    if (IS_BIT_SET(bitmap[i / 8], i % 8)) {
      return i;
    }
    ++i;
  }
  return -1;
}

static void kSetFlagInBitmap(int block_level, int offset, uint8 flag) {
  uint8* bitmap = dynamic_memory.bitmap_of_level[block_level].bitmap;
  if (flag == DYNAMIC_MEMORY_EXIST) {
    if (!IS_BIT_SET(bitmap[offset / 8], offset % 8)) {
      ++dynamic_memory.bitmap_of_level[block_level].exist_bit_count;
      SET_BIT(bitmap[offset / 8], offset % 8);
    }
  } else {
    if (IS_BIT_SET(bitmap[offset / 8], offset % 8)) {
      --dynamic_memory.bitmap_of_level[block_level].exist_bit_count;
      CLEAR_BIT(bitmap[offset / 8], offset % 8);
    }
  }
}

static bool kFreeBuddyBlock(int block_level, int block_offset) {
  const bool prev_interrupt_flag = kLockForSystemData();
  for (int i = block_level; i < dynamic_memory.max_level; ++i) {
    kSetFlagInBitmap(i, block_offset, DYNAMIC_MEMORY_EXIST);

    int buddy_block_offset;
    if (block_offset % 2 == 0) {
      buddy_block_offset = block_offset + 1;
    } else {
      buddy_block_offset = block_offset - 1;
    }

    const bool flag = kGetFlagInBitmap(i, buddy_block_offset);
    if (flag == DYNAMIC_MEMORY_EMPTY) {
      break;
    }

    kSetFlagInBitmap(i, buddy_block_offset, DYNAMIC_MEMORY_EMPTY);
    kSetFlagInBitmap(i, block_offset, DYNAMIC_MEMORY_EMPTY);
    block_offset /= 2;
  }
  kUnlockForSystemData(prev_interrupt_flag);
  return true;
}

inline static uint8 kGetFlagInBitmap(int block_level, int offset) {
  uint8* bitmap = dynamic_memory.bitmap_of_level[block_level].bitmap;
  if (IS_BIT_SET(bitmap[offset / 8], offset % 8)) {
    return DYNAMIC_MEMORY_EXIST;
  }
  return DYNAMIC_MEMORY_EMPTY;
}

inline void kGetDynamicMemoryInformation(uint64* dynamic_memory_start_addr,
                                         size_t* dynamic_memory_total_size,
                                         size_t* meta_data_size,
                                         size_t* used_memory_size) {
  *dynamic_memory_start_addr = (uint64)DYNAMIC_MEMORY_START_ADDRESS;
  const size_t mem_size = kCalculateDynamicMemorySize();
  *dynamic_memory_total_size = mem_size;
  *meta_data_size =
      kCalculateMetaBlockCount(mem_size) * DYNAMIC_MEMORY_MIN_SIZE;
  *used_memory_size = dynamic_memory.used_size;
}

inline DynamicMemory* kGetDynamicMemoryManager(void) { return &dynamic_memory; }