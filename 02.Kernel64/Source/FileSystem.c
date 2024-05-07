#include "FileSystem.h"

#include "HardDisk.h"
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

  uint32 total_sector_count = hdd_info->total_sectors;
  uint32 max_cluster_count =
      total_sector_count / FILESYSTEM_SECTORS_PER_CLUSTER;
  // Since link data is 4 bytes, 128 sectors can be located on one sector.
  uint32 cluster_link_sector_count = (max_cluster_count + 127) / 128;
  uint32 remain_sector_count =
      total_sector_count - cluster_link_sector_count - 1;
  uint32 cluster_count = remain_sector_count / FILESYSTEM_SECTORS_PER_CLUSTER;

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

inline bool kReadClusterLinkTable(uint32 offset, uint8* buf) {
  return read_hdd_sector(
      true, true, offset + filesystem_manager.cluster_link_area_start_address,
      1, buf);
}

inline bool kWriteClusterLinkTable(uint32 offset, uint8* buf) {
  return write_hdd_sector(
      true, true, offset + filesystem_manager.cluster_link_area_start_address,
      1, buf);
}

inline bool kReadCluster(uint32 offset, uint8* buf) {
  return read_hdd_sector(true, true,
                         offset * FILESYSTEM_SECTORS_PER_CLUSTER +
                             filesystem_manager.data_area_start_address,
                         FILESYSTEM_SECTORS_PER_CLUSTER, buf);
}

inline bool kWriteCluster(uint32 offset, uint8* buf) {
  return write_hdd_sector(true, true,
                          offset * FILESYSTEM_SECTORS_PER_CLUSTER +
                              filesystem_manager.data_area_start_address,
                          FILESYSTEM_SECTORS_PER_CLUSTER, buf);
}

uint32 kFindFreeCluster(void) {
  if (!filesystem_manager.mounted) {
    return FILESYSTEM_LAST_CLUSTER;
  }

  uint32 last_sector_offset =
      filesystem_manager.last_allocated_cluster_link_sector_offset;
  uint32 link_count_in_sector;
  for (int i = 0; i < filesystem_manager.cluster_link_area_size; ++i) {
    if (last_sector_offset + i ==
        filesystem_manager.cluster_link_area_size - 1) {
      link_count_in_sector = filesystem_manager.total_cluster_count % 128;
    } else {
      link_count_in_sector = 128;
    }

    uint32 current_sector_offset =
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

bool kSetClusterLinkData(uint32 cluster_index, uint32 data) {
  if (!filesystem_manager.mounted) {
    return false;
  }

  uint32 sector_offset = cluster_index / 128;
  if (kReadClusterLinkTable(sector_offset, temp_buffer) == false) {
    return false;
  }

  ((uint32*)temp_buffer)[cluster_index % 128] = data;

  if (kWriteClusterLinkTable(sector_offset, temp_buffer) == false) {
    return false;
  }
  return true;
}

bool kGetClusterLinkData(uint32 cluster_index, uint32* data) {
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

int kFindFreeDirectoryEntry(void) {
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

bool kSetDirectoryEntryData(int index, DirectoryEntry* entry) {
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

bool kGetDirectoryEntryData(int index, DirectoryEntry* entry) {
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

int kFindDirectoryEntry(const char* file_name, DirectoryEntry* entry) {
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