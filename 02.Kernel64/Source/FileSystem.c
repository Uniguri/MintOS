#include "FileSystem.h"

#include "DynamicMemory.h"
#include "HardDisk.h"
#include "Macro.h"
#include "Memory.h"
#include "String.h"
#include "Synchronization.h"
#include "Types.h"

ReadHDDInformation read_hdd_information = null;
ReadHDDSector read_hdd_sector = null;
WriteHDDSector write_hdd_sector = null;

static FileSystemManager filesystem_manager;

bool kInitializeFileSystem(void) {
  memset(&filesystem_manager, 0, sizeof(filesystem_manager));
  kInitializeMutex(&filesystem_manager.mutex);

  if (kInitializeHDD()) {
    read_hdd_information = kReadHDDInformation;
    read_hdd_sector = kReadHDDSector;
    write_hdd_sector = kWriteHDDSector;
  } else {
    return false;
  }

  if (kMount() == false) {
    return false;
  }

  filesystem_manager.handle_pool =
      (FILE*)kAllocateMemory(FILESYSTEM_HANDLE_MAX_COUNT * sizeof(FILE));
  if (filesystem_manager.handle_pool == nullptr) {
    filesystem_manager.mounted = false;
    return false;
  }

  memset(filesystem_manager.handle_pool, 0,
         FILESYSTEM_HANDLE_MAX_COUNT * sizeof(FILE));

  return true;
}

static uint8 temp_buffer[FILESYSTEM_SECTORS_PER_CLUSTER * 512];

bool kFormat(void) {
  kLockMutex(&filesystem_manager.mutex);

  HDDInformation* hdd_info = (HDDInformation*)temp_buffer;
  if (read_hdd_information(true, true, hdd_info) == false) {
    kUnlockMutex(&filesystem_manager.mutex);
    return false;
  }

  const uint32 total_sector_count = hdd_info->total_sectors;
  const uint32 max_cluster_count =
      total_sector_count / FILESYSTEM_SECTORS_PER_CLUSTER;
  // Since link data is 4 bytes, 128 sectors can be located on one sector.
  uint32 cluster_link_sector_count = (max_cluster_count + 127) / 128;
  const uint32 remain_sector_count =
      total_sector_count - cluster_link_sector_count - 1;
  const uint32 cluster_count =
      remain_sector_count / FILESYSTEM_SECTORS_PER_CLUSTER;

  // Since link data is 4 bytes, 128 sectors can be located on one sector.
  cluster_link_sector_count = (cluster_count + 127) / 128;

  if (read_hdd_sector(true, true, 0, 1, temp_buffer) == false) {
    kUnlockMutex(&filesystem_manager.mutex);
    return false;
  }

  MBR* mbr = (MBR*)temp_buffer;
  memset(mbr->partition, 0, sizeof(mbr->partition));
  mbr->signature = FILESYSTEM_SIGNATURE;
  mbr->reserved_sector_count = 0;
  mbr->cluster_link_sector_count = cluster_link_sector_count;
  mbr->total_cluster_count = cluster_count;

  if (write_hdd_sector(true, true, 0, 1, temp_buffer) == false) {
    kUnlockMutex(&filesystem_manager.mutex);
    return false;
  }

  memset(temp_buffer, 0, sizeof(temp_buffer));
  for (int i = 0;
       i < cluster_link_sector_count + FILESYSTEM_SECTORS_PER_CLUSTER; ++i) {
    if (i == 0) {
      ((uint32*)temp_buffer)[0] = FILESYSTEM_LAST_CLUSTER;
    } else {
      ((uint32*)temp_buffer)[0] = FILESYSTEM_FREE_CLUSTER;
    }

    if (write_hdd_sector(true, true, i + 1, 1, temp_buffer) == false) {
      kUnlockMutex(&filesystem_manager.mutex);
      return false;
    }
  }

  kUnlockMutex(&filesystem_manager.mutex);
  return true;
}

bool kMount(void) {
  kLockMutex(&filesystem_manager.mutex);
  if (read_hdd_sector(true, true, 0, 1, temp_buffer) == false) {
    kUnlockMutex(&filesystem_manager.mutex);
    return false;
  }

  MBR* mbr = (MBR*)temp_buffer;
  if (mbr->signature != FILESYSTEM_SIGNATURE) {
    kUnlockMutex(&filesystem_manager.mutex);
    return false;
  }

  filesystem_manager.mounted = true;

  filesystem_manager.reserved_sector_count = mbr->reserved_sector_count;
  filesystem_manager.cluster_link_area_start_address =
      mbr->reserved_sector_count + 1;
  filesystem_manager.cluster_link_area_size = mbr->cluster_link_sector_count;
  filesystem_manager.data_area_start_address =
      mbr->reserved_sector_count + mbr->cluster_link_sector_count + 1;
  filesystem_manager.total_cluster_count = mbr->total_cluster_count;

  kUnlockMutex(&filesystem_manager.mutex);
  return true;
}

inline bool kGetHDDInformation(HDDInformation* hdd_info) {
  kLockMutex(&filesystem_manager.mutex);
  const bool result = read_hdd_information(true, true, hdd_info);
  kUnlockMutex(&filesystem_manager.mutex);
  return result;
}

static inline bool kReadClusterLinkTable(uint32 offset, uint8* buf) {
  return read_hdd_sector(
      true, true, offset + filesystem_manager.cluster_link_area_start_address,
      1, buf);
}

static inline bool kWriteClusterLinkTable(uint32 offset, uint8* buf) {
  return write_hdd_sector(
      true, true, offset + filesystem_manager.cluster_link_area_start_address,
      1, buf);
}

static inline bool kReadCluster(uint32 offset, uint8* buf) {
  return read_hdd_sector(true, true,
                         offset * FILESYSTEM_SECTORS_PER_CLUSTER +
                             filesystem_manager.data_area_start_address,
                         FILESYSTEM_SECTORS_PER_CLUSTER, buf);
}

static inline bool kWriteCluster(uint32 offset, uint8* buf) {
  return write_hdd_sector(true, true,
                          offset * FILESYSTEM_SECTORS_PER_CLUSTER +
                              filesystem_manager.data_area_start_address,
                          FILESYSTEM_SECTORS_PER_CLUSTER, buf);
}

static uint32 kFindFreeCluster(void) {
  if (!filesystem_manager.mounted) {
    return FILESYSTEM_LAST_CLUSTER;
  }

  const uint32 last_sector_offset =
      filesystem_manager.last_allocated_cluster_link_sector_offset;
  uint32 link_count_in_sector;
  for (int i = 0; i < filesystem_manager.cluster_link_area_size; ++i) {
    if (last_sector_offset + i ==
        filesystem_manager.cluster_link_area_size - 1) {
      link_count_in_sector = filesystem_manager.total_cluster_count % 128;
    } else {
      link_count_in_sector = 128;
    }

    const uint32 current_sector_offset =
        (last_sector_offset + i) % filesystem_manager.cluster_link_area_size;
    if (kReadClusterLinkTable(current_sector_offset, temp_buffer) == false) {
      return FILESYSTEM_LAST_CLUSTER;
    }

    int j;
    for (j = 0; j < link_count_in_sector; ++j) {
      if (((uint32*)temp_buffer)[j] == FILESYSTEM_FREE_CLUSTER) {
        break;
      }
    }

    if (j != link_count_in_sector) {
      filesystem_manager.last_allocated_cluster_link_sector_offset =
          current_sector_offset;
      return (current_sector_offset * 128) + j;
    }
  }
  return FILESYSTEM_LAST_CLUSTER;
}

static bool kSetClusterLinkData(uint32 cluster_index, uint32 data) {
  if (!filesystem_manager.mounted) {
    return false;
  }

  const uint32 sector_offset = cluster_index / 128;
  if (kReadClusterLinkTable(sector_offset, temp_buffer) == false) {
    return false;
  }

  ((uint32*)temp_buffer)[cluster_index % 128] = data;

  if (kWriteClusterLinkTable(sector_offset, temp_buffer) == false) {
    return false;
  }
  return true;
}

static bool kGetClusterLinkData(uint32 cluster_index, uint32* data) {
  if (!filesystem_manager.mounted) {
    return false;
  }

  uint32 sector_offset = cluster_index / 128;
  if (sector_offset > filesystem_manager.cluster_link_area_size) {
    return false;
  }

  if (kReadClusterLinkTable(sector_offset, temp_buffer) == false) {
    return false;
  }

  *data = ((uint32*)temp_buffer)[cluster_index % 128];
  return true;
}

static int kFindFreeDirectoryEntry(void) {
  if (!filesystem_manager.mounted) {
    return -1;
  }
  if (kReadCluster(0, temp_buffer) == false) {
    return -1;
  }

  DirectoryEntry* entry = (DirectoryEntry*)temp_buffer;
  for (int i = 0; i < FILESYSTEM_MAX_DIRECTORY_ENTRY_COIUNT; ++i) {
    if (entry[i].start_cluster_index == 0) {
      return i;
    }
  }
  return -1;
}

static bool kSetDirectoryEntryData(int index, DirectoryEntry* entry) {
  if (!filesystem_manager.mounted || index < 0 ||
      index >= FILESYSTEM_MAX_DIRECTORY_ENTRY_COIUNT) {
    return false;
  }
  if (kReadCluster(0, temp_buffer) == false) {
    return false;
  }

  DirectoryEntry* root_entry = (DirectoryEntry*)temp_buffer;
  memcpy(root_entry + index, entry, sizeof(DirectoryEntry));

  if (kWriteCluster(0, temp_buffer) == false) {
    return false;
  }
  return true;
}

static bool kGetDirectoryEntryData(int index, DirectoryEntry* entry) {
  if (!filesystem_manager.mounted || index < 0 ||
      index >= FILESYSTEM_MAX_DIRECTORY_ENTRY_COIUNT) {
    return false;
  }
  if (kReadCluster(0, temp_buffer) == false) {
    return false;
  }

  DirectoryEntry* root_entry = (DirectoryEntry*)temp_buffer;
  memcpy(entry, root_entry + index, sizeof(DirectoryEntry));
  return true;
}

static int kFindDirectoryEntry(const char* file_name, DirectoryEntry* entry) {
  if (!filesystem_manager.mounted) {
    return -1;
  }
  if (kReadCluster(0, temp_buffer) == false) {
    return -1;
  }

  int len = strlen(file_name);
  DirectoryEntry* root_entry = (DirectoryEntry*)temp_buffer;
  for (int i = 0; i < FILESYSTEM_MAX_DIRECTORY_ENTRY_COIUNT; ++i) {
    if (memcmp(root_entry[i].file_name, file_name, len) == 0) {
      memcpy(entry, root_entry + i, sizeof(DirectoryEntry));
      return i;
    }
  }
  return -1;
}

inline void kGetFileSystemInformation(FileSystemManager* manager) {
  memcpy(manager, &filesystem_manager, sizeof(FileSystemManager));
}

FILE* kOpenFile(const char* file_name, const char* mode) {
  DirectoryEntry entry;

  const int file_name_length = strlen(file_name);
  if (file_name_length == 0 || file_name_length >= sizeof(entry.file_name)) {
    return nullptr;
  }

  kLockMutex(&filesystem_manager.mutex);

  int directory_entry_offset = kFindDirectoryEntry(file_name, &entry);
  int second_cluster;
  if (directory_entry_offset == -1) {
    if (mode[0] == 'r') {
      kUnlockMutex(&filesystem_manager.mutex);
      return nullptr;
    }
    if (kCreateFile(file_name, &entry, &directory_entry_offset) == false) {
      kUnlockMutex(&filesystem_manager.mutex);
      return nullptr;
    }
  } else if (mode[0] == 'w') {
    if (kGetClusterLinkData(entry.start_cluster_index, &second_cluster) ==
        false) {
      kUnlockMutex(&filesystem_manager.mutex);
      return nullptr;
    }

    if (kSetClusterLinkData(entry.start_cluster_index,
                            FILESYSTEM_LAST_CLUSTER) == false) {
      kUnlockMutex(&filesystem_manager.mutex);
      return nullptr;
    }

    if (kFreeClusterUntilEnd(second_cluster) == false) {
      kUnlockMutex(&filesystem_manager.mutex);
      return nullptr;
    }

    entry.file_size = 0;
    if (kSetDirectoryEntryData(directory_entry_offset, &entry) == false) {
      kUnlockMutex(&filesystem_manager.mutex);
      return nullptr;
    }
  }

  FILE* file = kAllocateFileDirectoryHandle();
  if (!file) {
    kUnlockMutex(&filesystem_manager.mutex);
    return nullptr;
  }

  file->type = FILESYSTEM_TYPE_FILE;
  file->file_handle.directory_entry_offset = directory_entry_offset;
  file->file_handle.file_size = entry.file_size;
  file->file_handle.start_cluster_index = entry.start_cluster_index;
  file->file_handle.current_cluster_index = entry.start_cluster_index;
  file->file_handle.previous_cluster_index = entry.start_cluster_index;
  file->file_handle.current_offset = 0;

  if (mode[0] == 'a') {
    kSeekFile(file, 0, FILESYSTEM_SEEK_END);
  }

  kUnlockMutex(&filesystem_manager.mutex);
  return file;
}

uint32 kReadFile(void* buffer, uint32 size, uint32 count, FILE* file) {
  if (!file || file->type != FILESYSTEM_TYPE_FILE) {
    return 0;
  }

  FileHandle* file_handle = &file->file_handle;
  if (file_handle->current_offset == file_handle->file_size ||
      file_handle->current_cluster_index == FILESYSTEM_LAST_CLUSTER) {
    return 0;
  }

  const uint32 total_count =
      MIN(size * count, file_handle->file_size - file_handle->current_offset);
  kLockMutex(&filesystem_manager.mutex);

  uint32 read_count = 0;
  while (read_count != total_count) {
    if (kReadCluster(file_handle->current_cluster_index, temp_buffer) ==
        false) {
      break;
    }
    const uint32 offset_in_cluster =
        file_handle->current_offset % FILESYSTEM_CLUSTER_SIZE;
    const uint32 copy_size = MIN(FILESYSTEM_CLUSTER_SIZE - offset_in_cluster,
                                 total_count - read_count);
    memcpy((char*)buffer + read_count, temp_buffer + offset_in_cluster,
           copy_size);
    read_count += copy_size;
    file_handle->current_offset += copy_size;
    if (file_handle->current_offset % FILESYSTEM_CLUSTER_SIZE == 0) {
      uint32 next_cluster_idx;
      if (kGetClusterLinkData(file_handle->current_cluster_index,
                              &next_cluster_idx) == false) {
        break;
      }
      file_handle->previous_cluster_index = file_handle->current_cluster_index;
      file_handle->current_cluster_index = next_cluster_idx;
    }
  }

  kUnlockMutex(&filesystem_manager.mutex);
  return read_count / size;
}

uint32 kWriteFile(const void* buffer, uint32 size, uint32 count, FILE* file) {
  if (!file || file->type != FILESYSTEM_TYPE_FILE) {
    return 0;
  }
  FileHandle* file_handle = &file->file_handle;
  const uint32 total_count = size * count;

  kLockMutex(&filesystem_manager.mutex);

  uint32 write_count = 0;
  while (write_count != total_count) {
    uint32 allocated_cluster_idx;
    if (file_handle->current_cluster_index == FILESYSTEM_LAST_CLUSTER) {
      allocated_cluster_idx = kFindFreeCluster();
      if (allocated_cluster_idx == FILESYSTEM_LAST_CLUSTER) {
        break;
      }
      if (kSetClusterLinkData(allocated_cluster_idx, FILESYSTEM_LAST_CLUSTER) ==
          false) {
        break;
      }
      if (kSetClusterLinkData(file_handle->previous_cluster_index,
                              allocated_cluster_idx) == false) {
        kSetClusterLinkData(allocated_cluster_idx, FILESYSTEM_FREE_CLUSTER);
        break;
      }
      file_handle->current_cluster_index = allocated_cluster_idx;
      memset(temp_buffer, 0, sizeof(temp_buffer));
    } else if (file_handle->current_offset % FILESYSTEM_CLUSTER_SIZE != 0 ||
               total_count - write_count < FILESYSTEM_CLUSTER_SIZE) {
      if (kReadCluster(file_handle->current_cluster_index, temp_buffer) ==
          false) {
        break;
      }
    }

    const uint32 offset_in_cluster =
        file_handle->current_offset % FILESYSTEM_CLUSTER_SIZE;
    const uint32 copy_size = MIN(FILESYSTEM_CLUSTER_SIZE - offset_in_cluster,
                                 total_count - write_count);
    memcpy(temp_buffer + offset_in_cluster, (char*)buffer + write_count,
           copy_size);
    if (kWriteCluster(file_handle->current_cluster_index, temp_buffer) ==
        false) {
      break;
    }
    write_count += copy_size;
    file_handle->current_offset += copy_size;
    uint32 next_cluster_idx;
    if (file_handle->current_offset % FILESYSTEM_CLUSTER_SIZE == 0) {
      if (kGetClusterLinkData(file_handle->current_cluster_index,
                              &next_cluster_idx) == false) {
        break;
      }
      file_handle->previous_cluster_index = file_handle->current_cluster_index;
      file_handle->current_cluster_index = next_cluster_idx;
    }
  }

  if (file_handle->file_size < file_handle->current_offset) {
    file_handle->file_size = file_handle->current_offset;
    kUpdateDirectoryEntry(file_handle);
  }

  kUnlockMutex(&filesystem_manager.mutex);
  return write_count / size;
}

int kSeekFile(FILE* file, int offset, int origin) {
  if (!file || file->type != FILESYSTEM_TYPE_FILE) {
    return 0;
  }

  FileHandle* file_handle = &file->file_handle;
  uint32 real_offset;
  switch (origin) {
    case FILESYSTEM_SEEK_SET:
      if (offset <= 0) {
        real_offset = 0;
      } else {
        real_offset = offset;
      }
      break;
    case FILESYSTEM_SEEK_CUR:
      if (offset < 0 && file_handle->current_offset <= (uint32)-offset) {
        real_offset = 0;
      } else {
        real_offset = file_handle->current_offset + offset;
      }
      break;
    case FILESYSTEM_SEEK_END:
      if (offset < 0 && file_handle->file_size <= (uint32)-offset) {
        real_offset = 0;
      } else {
        real_offset = file_handle->file_size + offset;
      }
      break;
    default:
      return 0;
  }
  const uint32 last_cluster_offset =
      file_handle->file_size / FILESYSTEM_CLUSTER_SIZE;
  const uint32 cluster_offset_to_move = real_offset / FILESYSTEM_CLUSTER_SIZE;
  uint32 current_cluster_offset =
      file_handle->current_offset / FILESYSTEM_CLUSTER_SIZE;
  uint32 move_count;
  uint32 start_cluster_idx;
  if (last_cluster_offset < cluster_offset_to_move) {
    move_count = last_cluster_offset - current_cluster_offset;
    start_cluster_idx = file_handle->current_cluster_index;
  } else if (current_cluster_offset <= cluster_offset_to_move) {
    move_count = cluster_offset_to_move - current_cluster_offset;
    start_cluster_idx = file_handle->current_cluster_index;
  } else {
    move_count = cluster_offset_to_move;
    start_cluster_idx = file_handle->start_cluster_index;
  }

  kLockMutex(&filesystem_manager.mutex);

  uint32 previous_cluster_idx;
  current_cluster_offset = start_cluster_idx;
  for (int i = 0; i < move_count; ++i) {
    previous_cluster_idx = current_cluster_offset;
    if (kGetClusterLinkData(previous_cluster_idx, &current_cluster_offset) ==
        false) {
      kUnlockMutex(&filesystem_manager.mutex);
      return -1;
    }
  }

  if (move_count > 0) {
    file_handle->previous_cluster_index = previous_cluster_idx;
    file_handle->current_cluster_index = current_cluster_offset;
  } else if (start_cluster_idx == file_handle->start_cluster_index) {
    file_handle->previous_cluster_index = file_handle->start_cluster_index;
    file_handle->current_cluster_index = file_handle->start_cluster_index;
  }

  if (last_cluster_offset < cluster_offset_to_move) {
    file_handle->current_offset = file_handle->file_size;
    kUnlockMutex(&filesystem_manager.mutex);
    if (kWriteZero(file, real_offset - file_handle->file_size) == false) {
      return 0;
    }
  }

  file_handle->current_offset = real_offset;
  kUnlockMutex(&filesystem_manager.mutex);
  return 0;
}

inline int kCloseFile(FILE* file) {
  if (!file || file->type != FILESYSTEM_TYPE_FILE) {
    return -1;
  }
  kFreeFileDirectoryHandle(file);
  return 0;
}

int kRemoveFile(const char* file_name) {
  DirectoryEntry entry;

  const int file_name_length = strlen(file_name);
  if (file_name_length == 0 || file_name_length > sizeof(entry.file_name)) {
    return 0;
  }

  kLockMutex(&filesystem_manager.mutex);

  uint32 directory_entry_offset = kFindDirectoryEntry(file_name, &entry);
  if (directory_entry_offset == -1) {
    kUnlockMutex(&filesystem_manager.mutex);
    return -1;
  }

  if (kIsFileOpened(&entry) == true) {
    kUnlockMutex(&filesystem_manager.mutex);
    return -1;
  }
  if (kFreeClusterUntilEnd(entry.start_cluster_index) == false) {
    kUnlockMutex(&filesystem_manager.mutex);
    return -1;
  }

  memset(&entry, 0, sizeof(entry));
  if (kSetDirectoryEntryData(directory_entry_offset, &entry) == false) {
    kUnlockMutex(&filesystem_manager.mutex);
    return -1;
  }
  kUnlockMutex(&filesystem_manager.mutex);
  return 0;
}

DIR* kOpenDirectory(const char* directory_name) {
  kLockMutex(&filesystem_manager.mutex);
  DIR* directory = kAllocateFileDirectoryHandle();
  if (!directory) {
    kUnlockMutex(&filesystem_manager.mutex);
    return nullptr;
  }

  DirectoryEntry* directory_buffer =
      (DirectoryEntry*)kAllocateMemory(FILESYSTEM_CLUSTER_SIZE);
  if (!directory_buffer) {
    kFreeFileDirectoryHandle(directory);
    kUnlockMutex(&filesystem_manager.mutex);
    return nullptr;
  }

  if (kReadCluster(0, (uint8*)directory_buffer) == false) {
    kFreeFileDirectoryHandle(directory);
    kFreeMemory(directory_buffer);
    kUnlockMutex(&filesystem_manager.mutex);
    return nullptr;
  }

  directory->type = FILESYSTEM_TYPE_DIRECTORY;
  directory->directory_handle.current_offset = 0;
  directory->directory_handle.directory_buffer = directory_buffer;

  kUnlockMutex(&filesystem_manager.mutex);
  return directory;
}

DirectoryEntry* kReadDirectory(DIR* directory) {
  if (!directory || directory->type != FILESYSTEM_TYPE_DIRECTORY) {
    return nullptr;
  }

  DirectoryHandle* directory_handle = &directory->directory_handle;
  if (directory_handle->current_offset < 0 ||
      directory_handle->current_offset >=
          FILESYSTEM_MAX_DIRECTORY_ENTRY_COIUNT) {
    return nullptr;
  }

  kLockMutex(&filesystem_manager.mutex);

  DirectoryEntry* entry = directory_handle->directory_buffer;
  while (directory_handle->current_offset <
         FILESYSTEM_MAX_DIRECTORY_ENTRY_COIUNT) {
    if (entry[directory_handle->current_offset].start_cluster_index != 0) {
      kUnlockMutex(&filesystem_manager.mutex);
      return &entry[directory_handle->current_offset++];
    }
    ++directory_handle->current_offset;
  }
  kUnlockMutex(&filesystem_manager.mutex);
  return nullptr;
}

inline void kRewindDirectory(DIR* directory) {
  if (!directory || directory->type != FILESYSTEM_TYPE_DIRECTORY) {
    return;
  }
  DirectoryHandle* directory_handle = &directory->directory_handle;
  kLockMutex(&filesystem_manager.mutex);
  directory_handle->current_offset = 0;
  kUnlockMutex(&filesystem_manager.mutex);
}

int kCloseDirectory(DIR* directory) {
  if (!directory || directory->type != FILESYSTEM_TYPE_DIRECTORY) {
    return -1;
  }

  DirectoryHandle* directory_handle = &directory->directory_handle;
  kLockMutex(&filesystem_manager.mutex);

  kFreeMemory(directory_handle->directory_buffer);
  kFreeFileDirectoryHandle(directory);

  kUnlockMutex(&filesystem_manager.mutex);
  return 0;
}

bool kWriteZero(FILE* file, uint32 count) {
  if (!file) {
    return false;
  }

  uint8* buffer = (uint8*)kAllocateMemory(FILESYSTEM_CLUSTER_SIZE);
  if (!buffer) {
    return false;
  }

  memset(buffer, 0, FILESYSTEM_CLUSTER_SIZE);
  uint32 remain_count = count;
  while (remain_count != 0) {
    uint32 write_count = MIN(remain_count, FILESYSTEM_CLUSTER_SIZE);
    if (kWriteFile(buffer, 1, write_count, file) != write_count) {
      kFreeMemory(buffer);
      return false;
    }
    remain_count -= write_count;
  }
  kFreeMemory(buffer);
  return true;
}

bool kIsFileOpened(const DirectoryEntry* entry) {
  FILE* file = filesystem_manager.handle_pool;
  for (int i = 0; i < FILESYSTEM_HANDLE_MAX_COUNT; ++i) {
    if (file[i].type == FILESYSTEM_TYPE_FILE &&
        file[i].file_handle.start_cluster_index == entry->start_cluster_index) {
      return true;
    }
  }
  return false;
}

static void* kAllocateFileDirectoryHandle(void) {
  FILE* file = filesystem_manager.handle_pool;
  for (int i = 0; i < FILESYSTEM_HANDLE_MAX_COUNT; ++i) {
    if (file->type == FILESYSTEM_TYPE_FREE) {
      file->type = FILESYSTEM_TYPE_FILE;
      return file;
    }
    ++file;
  }
  return nullptr;
}

inline static void kFreeFileDirectoryHandle(FILE* file) {
  memset(file, 0, sizeof(FILE));
  file->type = FILESYSTEM_TYPE_FREE;
}

static bool kCreateFile(const char* file_name, DirectoryEntry* entry,
                        int* directory_entry_index) {
  uint32 cluster = kFindFreeCluster();
  if (cluster == FILESYSTEM_LAST_CLUSTER ||
      kSetClusterLinkData(cluster, FILESYSTEM_LAST_CLUSTER) == false) {
    return false;
  }

  *directory_entry_index = kFindFreeDirectoryEntry();
  if (*directory_entry_index == -1) {
    kSetClusterLinkData(cluster, FILESYSTEM_FREE_CLUSTER);
    return false;
  }

  memcpy(entry->file_name, file_name, strlen(file_name));
  entry->start_cluster_index = cluster;
  entry->file_size = 0;

  if (kSetDirectoryEntryData(*directory_entry_index, entry) == false) {
    kSetClusterLinkData(cluster, FILESYSTEM_FREE_CLUSTER);
    return false;
  }

  return true;
}

static bool kFreeClusterUntilEnd(uint32 cluster_index) {
  uint32 cur_cluster_idx = cluster_index;
  while (cur_cluster_idx != FILESYSTEM_LAST_CLUSTER) {
    uint32 next_cluster_idx;
    if (kGetClusterLinkData(cur_cluster_idx, &next_cluster_idx) == false) {
      return false;
    }

    if (kSetClusterLinkData(cur_cluster_idx, FILESYSTEM_FREE_CLUSTER) ==
        false) {
      return false;
    }
    cur_cluster_idx = next_cluster_idx;
  }
  return true;
}

static bool kUpdateDirectoryEntry(FileHandle* file_handle) {
  DirectoryEntry entry;
  if (!file_handle ||
      kGetDirectoryEntryData(file_handle->directory_entry_offset, &entry) ==
          false) {
    return false;
  }
  entry.file_size = file_handle->file_size;
  entry.start_cluster_index = file_handle->start_cluster_index;
  if (kSetDirectoryEntryData(file_handle->directory_entry_offset, &entry) ==
      false) {
    return false;
  }
  return true;
}