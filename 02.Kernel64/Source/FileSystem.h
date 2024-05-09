#ifndef MINTOS_DYNAMIC_FILESYSTEM_H_
#define MINTOS_DYNAMIC_FILESYSTEM_H_

#include "HardDisk.h"
#include "Synchronization.h"
#include "Task.h"
#include "Types.h"

#define FILESYSTEM_SIGNATURE (0x7E38CF10)

#define FILESYSTEM_SECTORS_PER_CLUSTER (8)
#define FILESYSTEM_LAST_CLUSTER (0xFFFFFFFF)
#define FILESYSTEM_FREE_CLUSTER (0x00)
#define FILESYSTEM_CLUSTER_SIZE (FILESYSTEM_SECTORS_PER_CLUSTER * 512)

#define FILESYSTEM_MAX_DIRECTORY_ENTRY_COIUNT \
  ((FILESYSTEM_SECTORS_PER_CLUSTER * 512) / sizeof(DirectoryEntry))

#define FILESYSTEM_HANDLE_MAX_COUNT (TASK_MAX_COUNT * 3)

#define FILESYSTEM_MAX_FILE_NAME_LENGTH (24)

#define FILESYSTEM_TYPE_FREE (0)
#define FILESYSTEM_TYPE_FILE (1)
#define FILESYSTEM_TYPE_DIRECTORY (2)

#define FILESYSTEM_SEEK_SET 0
#define FILESYSTEM_SEEK_CUR 1
#define FILESYSTEM_SEEK_END 2

typedef bool (*ReadHDDInformation)(bool primary, bool master,
                                   HDDInformation* hdd_info);
typedef int (*ReadHDDSector)(bool primary, bool master, uint32 lba,
                             int sector_count, char* buf);
typedef int (*WriteHDDSector)(bool primary, bool master, uint32 lba,
                              int sector_count, const char* buffer);

#define fopen kOpenFile
#define fread kReadFile
#define fwrite kWriteFile
#define fseek kSeekFile
#define fclose kCloseFile
#define remove kRemoveFile
#define opendir kOpenDirectory
#define readdir kReadDirectory
#define rewinddir kRewindDirectory
#define closedir kCloseDirectory

#define SEEK_SET FILESYSTEM_SEEK_SET
#define SEEK_CUR FILESYSTEM_SEEK_CUR
#define SEEK_END FILESYSTEM_SEEK_END

#define dirent kDirectoryEntryStruct
#define d_name file_name

#pragma pack(push, 1)
typedef struct kPartitionStruct {
  uint8 bootable_flag;
  uint8 starting_chs_address[3];
  uint8 partition_type;
  uint8 ending_chs_address[3];
  uint32 starting_lba_address;
  uint32 size_in_sector;
} Partition;

typedef struct kMBRStruct {
  uint8 boot_code[430];
  // File system signature; 0x7E38CF10
  uint32 signature;
  uint32 reserved_sector_count;
  uint32 cluster_link_sector_count;
  uint32 total_cluster_count;

  Partition partition[4];

  uint8 boot_loader_signature[2];
} MBR;

typedef struct kDirectoryEntryStruct {
  char file_name[FILESYSTEM_MAX_FILE_NAME_LENGTH];
  uint32 file_size;
  uint32 start_cluster_index;
} DirectoryEntry;
#pragma pack(pop)

typedef struct kFileHandleStruct {
  int directory_entry_offset;
  uint32 file_size;
  uint32 start_cluster_index;
  uint32 current_cluster_index;
  uint32 previous_cluster_index;
  uint32 current_offset;
} FileHandle;

typedef struct kDirectoryHandleStruct {
  DirectoryEntry* directory_buffer;

  int current_offset;
} DirectoryHandle;

typedef struct kFileDirectoryHandleStruct {
  uint8 type;

  union {
    FileHandle file_handle;
    DirectoryHandle directory_handle;
  };
} FILE, DIR;
typedef struct kFileSystemManagerStruct {
  bool mounted;

  uint32 reserved_sector_count;
  uint32 cluster_link_area_start_address;
  uint32 cluster_link_area_size;
  uint32 data_area_start_address;

  uint32 total_cluster_count;

  uint32 last_allocated_cluster_link_sector_offset;

  Mutex mutex;

  FILE* handle_pool;
} FileSystemManager;

// Initialize HDD and Filesystem and then mount it if there is an Mint
// filesystem hdd.
// @return true if initialization and mounting success.
bool kInitializeFileSystem(void);

// Format HDD with Mint filesystem.
// @return true if formatting successes.
bool kFormat(void);

// Mount HDD formatted by Mint filesystem.
// @return true if mounting successes.
bool kMount(void);

// Get mounted HDD's information.
// @param hdd_info: pointer to store hdd information.
// @return true if hdd_info is set by valid value.
bool kGetHDDInformation(HDDInformation* hdd_info);

// Read cluster link table.
// @param offset: offset of cluster link offset to read.
// @param buf: pointer to store read sector(512 bytes).
// @return true if reading successes.
static bool kReadClusterLinkTable(uint32 offset, uint8* buf);

// Write cluster link table.
// @param offset: offset of cluster link offset to set.
// @param buf: pointer to data(one sector; 512 bytes) to write.
// @return true if reading successes.
static bool kWriteClusterLinkTable(uint32 offset, uint8* buf);

// Read cluster.
// @param offset: offset of cluster to read.
// @param buf: pointer to store read cluster(8 sectors; 4096 bytes).
// @return true if reading successes.
static bool kReadCluster(uint32 offset, uint8* buf);

// Write cluster.
// @param offset: offset of clusterto set.
// @param buf: pointer to cluster(8 sectors; 4096 bytes) to write.
// @return true if reading successes.
static bool kWriteCluster(uint32 offset, uint8* buf);

// Find not used cluster.
// @return valid cluster index when success, otherwise FILESYSTEM_LASTCLUSTER.
static uint32 kFindFreeCluster(void);

// Set cluster link data.
// @param cluster_index: index of cluster to set.
// @param data: data to set.
// @return true if success.
static bool kSetClusterLinkData(uint32 cluster_index, uint32 data);

// Get cluster link data.
// @param cluster_index: index of cluster to get.
// @param data: pointer to uint32 to store data.
// @return true if success.
static bool kGetClusterLinkData(uint32 cluster_index, uint32* data);

// Find not used directory entry.
// @return value in [0, FILESYSTEM_MAX_DIRECTORY_ENTRY_COIUNT] when successs,
// otherwise -1.
static int kFindFreeDirectoryEntry(void);

// Set directory entry data.
// @param index: index of directory entry to set.
// @param entry: pointer to data to set.
// @return true if success.
static bool kSetDirectoryEntryData(int index, DirectoryEntry* entry);

// Get directory entry data.
// @param index: index of directory entry to get.
// @param entry: pointer to data to store data.
// @return true if success.
static bool kGetDirectoryEntryData(int index, DirectoryEntry* entry);

// Find directory entry having given file name.
// @param file_name: file name to find directory entry.
// @param entry: pointer to store found directory entry.
// @return: index of directory entry when found, otherwise -1.
static int kFindDirectoryEntry(const char* file_name, DirectoryEntry* entry);

// Get information of filesystem.
// @param manager: pointer to store information.
void kGetFileSystemInformation(FileSystemManager* manager);

FILE* kOpenFile(const char* file_name, const char* mode);

uint32 kReadFile(void* buffer, uint32 size, uint32 count, FILE* file);

uint32 kWriteFile(const void* buffer, uint32 size, uint32 count, FILE* file);

int kSeekFile(FILE* file, int offset, int origin);

int kCloseFile(FILE* file);

int kRemoveFile(const char* file_name);

DIR* kOpenDirectory(const char* directory_name);

DirectoryEntry* kReadDirectory(DIR* directory);

void kRewindDirectory(DIR* directory);

int kCloseDirectory(DIR* directory);

bool kWriteZero(FILE* file, uint32 count);

bool kIsFileOpened(const DirectoryEntry* entry);

static void* kAllocateFileDirectoryHandle(void);

static void kFreeFileDirectoryHandle(FILE* file);

static bool kCreateFile(const char* file_name, DirectoryEntry* entry,
                        int* directory_entry_index);

static bool kFreeClusterUntilEnd(uint32 cluster_index);

static bool kUpdateDirectoryEntry(FileHandle* file_handle);

#endif