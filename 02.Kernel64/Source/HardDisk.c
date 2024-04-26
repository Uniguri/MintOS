#include "HardDisk.h"

#include "Hardware.h"
#include "Memory.h"
#include "Synchronization.h"
#include "Tick.h"
#include "Types.h"

static HddManager hdd_manager;

bool kInitializeHDD(void) {
  kInitializeMutex(&hdd_manager.mutex);

  hdd_manager.primary_interrupt_occur = false;
  hdd_manager.secondary_interrupt_occur = false;

  kSetPortByte(HDD_PORT_PRIMARY_BASE + HDD_PORT_INDEX_DEVICE_CONTROL, 0);
  kSetPortByte(HDD_PORT_SECONDARY_BASE + HDD_PORT_INDEX_DEVICE_CONTROL, 0);

  // Detect HDD and get informations. If we cannot read info, return false.
  if (!kReadHDDInformation(true, true, &hdd_manager.hdd_information)) {
    hdd_manager.hdd_detected = false;
    hdd_manager.can_write = false;
    return false;
  }

  hdd_manager.hdd_detected = true;
  // write is allowed only when disk is QEMU's virtual disk.
  if (!memcmp(hdd_manager.hdd_information.model_number, "QEMU", 4)) {
    hdd_manager.can_write = true;
  } else {
    hdd_manager.can_write = false;
  }
  return true;
}

inline void kSetHDDInterruptFlag(bool primary, bool flag) {
  if (primary) {
    hdd_manager.primary_interrupt_occur = flag;
  } else {
    hdd_manager.secondary_interrupt_occur = flag;
  }
}

bool kReadHDDInformation(bool primary, bool master, HDDInformation* hdd_info) {
  const uint16 port_base =
      primary ? HDD_PORT_PRIMARY_BASE : HDD_PORT_SECONDARY_BASE;

  kLockMutex(&hdd_manager.mutex);
  if (!kWaitForHDDNoBusy(primary)) {
    kUnlockMutex(&hdd_manager.mutex);
    return false;
  }

  const uint8 drive_flag =
      master ? HDD_DRIVE_AND_HEAD_LBA
             : (HDD_DRIVE_AND_HEAD_LBA | HDD_DRIVE_AND_HEAD_SLAVE);
  kSetPortByte(port_base + HDD_PORT_INDEX_DRIVE_AND_HEAD, drive_flag);

  if (!kWaitForHDDReady(primary)) {
    kUnlockMutex(&hdd_manager.mutex);
    return false;
  }

  kSetHDDInterruptFlag(primary, false);

  kSetPortByte(port_base + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_INDENTIFY);
  bool wait_result = kWaitForHDDInterrupt(primary);
  bool status = kReadHDDStatus(primary);
  if (!wait_result || status & HDD_STATUS_ERROR) {
    kUnlockMutex(&hdd_manager.mutex);
    return false;
  }

  for (int i = 0; i < 512 / 2; ++i) {
    ((uint16*)hdd_info)[i] = kGetPortWord(port_base + HDD_PORT_INDEX_DATA);
  }
  kSwapByteInWord(hdd_info->model_number, sizeof(hdd_info->model_number) / 2);
  kSwapByteInWord(hdd_info->serial_number, sizeof(hdd_info->serial_number) / 2);

  kUnlockMutex(&hdd_manager.mutex);
  return true;
}

int kReadHDDSector(bool primary, bool master, uint32 lba, int sector_count,
                   char* buffer) {
  if (!hdd_manager.hdd_detected || sector_count <= 0 || 256 < sector_count ||
      lba + sector_count >= hdd_manager.hdd_information.total_sectors) {
    return 0;
  }

  const uint16 port_base =
      primary ? HDD_PORT_PRIMARY_BASE : HDD_PORT_SECONDARY_BASE;
  const uint8 drive_flag =
      master ? HDD_DRIVE_AND_HEAD_LBA
             : (HDD_DRIVE_AND_HEAD_LBA | HDD_DRIVE_AND_HEAD_SLAVE);

  kLockMutex(&hdd_manager.mutex);
  if (!kWaitForHDDNoBusy(primary)) {
    kUnlockMutex(&hdd_manager.mutex);
    return false;
  }
  kSetPortByte(port_base + HDD_PORT_INDEX_SECTOR_COUNT, sector_count);
  kSetPortByte(port_base + HDD_PORT_INDEX_SECTOR_NUMBER, lba);
  kSetPortByte(port_base + HDD_PORT_INDEX_CYLINDER_LSB, lba >> 8);
  kSetPortByte(port_base + HDD_PORT_INDEX_CYLINDER_MSB, lba >> 16);
  kSetPortByte(port_base + HDD_PORT_INDEX_DRIVE_AND_HEAD,
               drive_flag | ((lba >> 24) & 0x0F));

  if (!kWaitForHDDReady(primary)) {
    kUnlockMutex(&hdd_manager.mutex);
    return false;
  }

  kSetHDDInterruptFlag(primary, false);

  kSetPortByte(port_base + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ);

  int cur_sector;
  int read_count = 0;
  for (cur_sector = 0; cur_sector < sector_count; ++cur_sector) {
    const uint8 status = kReadHDDStatus(primary);
    if (status & HDD_STATUS_ERROR) {
      kUnlockMutex(&hdd_manager.mutex);
      return cur_sector;
    }

    if (~status & HDD_STATUS_DATA_REQUEST) {
      const uint8 wait_result = kWaitForHDDInterrupt(primary);
      kSetHDDInterruptFlag(primary, false);
      if (!wait_result) {
        kUnlockMutex(&hdd_manager.mutex);
        return 0;
      }
    }

    for (int j = 0; j < 512 / 2; ++j) {
      ((uint16*)buffer)[read_count++] =
          kGetPortWord(port_base + HDD_PORT_INDEX_DATA);
    }
  }

  kUnlockMutex(&hdd_manager.mutex);
  return cur_sector;
}

int kWriteHDDSector(bool primary, bool master, uint32 lba, int sector_count,
                    const char* buffer) {
  if (!hdd_manager.can_write || sector_count <= 0 || 256 < sector_count ||
      lba + sector_count >= hdd_manager.hdd_information.total_sectors) {
    return 0;
  }

  if (!kWaitForHDDNoBusy(primary)) {
    return 0;
  }

  const uint16 port_base =
      primary ? HDD_PORT_PRIMARY_BASE : HDD_PORT_SECONDARY_BASE;
  const uint8 drive_flag =
      master ? HDD_DRIVE_AND_HEAD_LBA
             : (HDD_DRIVE_AND_HEAD_LBA | HDD_DRIVE_AND_HEAD_SLAVE);

  kLockMutex(&hdd_manager.mutex);
  kSetPortByte(port_base + HDD_PORT_INDEX_SECTOR_COUNT, sector_count);
  kSetPortByte(port_base + HDD_PORT_INDEX_SECTOR_NUMBER, lba);
  kSetPortByte(port_base + HDD_PORT_INDEX_CYLINDER_LSB, lba >> 8);
  kSetPortByte(port_base + HDD_PORT_INDEX_CYLINDER_MSB, lba >> 16);
  kSetPortByte(port_base + HDD_PORT_INDEX_DRIVE_AND_HEAD,
               drive_flag | ((lba >> 24) & 0x0F));

  if (!kWaitForHDDReady(primary)) {
    kUnlockMutex(&hdd_manager.mutex);
    return 0;
  }

  kSetPortByte(port_base + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE);
  uint16 write_count = 0;
  while (1) {
    const uint8 status = kReadHDDStatus(primary);
    if (status & HDD_STATUS_ERROR) {
      kUnlockMutex(&hdd_manager.mutex);
      return 0;
    }
    if (status & HDD_STATUS_DATA_REQUEST) {
      break;
    }
    kSleep(1);
  }

  int cur_sector;
  for (cur_sector = 0; cur_sector < sector_count; ++cur_sector) {
    kSetHDDInterruptFlag(primary, false);
    for (int j = 0; j < 512 / 2; ++j) {
      kSetPortWord(port_base + HDD_PORT_INDEX_DATA,
                   ((uint16*)buffer)[write_count++]);
    }

    const uint8 status = kReadHDDStatus(primary);
    if (status & HDD_STATUS_ERROR) {
      kUnlockMutex(&hdd_manager.mutex);
      return cur_sector;
    }

    if (~status & HDD_STATUS_DATA_REQUEST) {
      const bool wait_result = kWaitForHDDInterrupt(primary);
      kSetHDDInterruptFlag(primary, false);
      if (!wait_result) {
        kUnlockMutex(&hdd_manager.mutex);
        return 0;
      }
    }
  }

  kUnlockMutex(&hdd_manager.mutex);
  return cur_sector;
}

inline static uint8 kReadHDDStatus(bool primary) {
  if (primary) {
    return kGetPortByte(HDD_PORT_PRIMARY_BASE + HDD_PORT_INDEX_STATUS);
  }
  return kGetPortByte(HDD_PORT_SECONDARY_BASE + HDD_PORT_INDEX_STATUS);
}

static void kSwapByteInWord(uint16* data, int word_count) {
  for (int i = 0; i < word_count; ++i) {
    const uint16 temp = data[i];
    data[i] = (temp >> 8) | (temp << 8);
  }
}

static bool kWaitForHDDReady(bool primary) {
  const uint64 start_tick_count = kGetTickCount();
  while (kGetTickCount() - start_tick_count <= HDD_WAIT_TIME) {
    const bool status = kReadHDDStatus(primary);
    if (status & HDD_STATUS_READY) {
      return true;
    }
    kSleep(1);
  }
  return false;
}

static bool kWaitForHDDNoBusy(bool primary) {
  const uint64 start_tick_count = kGetTickCount();
  while (kGetTickCount() - start_tick_count <= HDD_WAIT_TIME) {
    const bool status = kReadHDDStatus(primary);
    if (~status & HDD_STATUS_BUSY) {
      return true;
    }
    kSleep(1);
  }
  return false;
}

static bool kWaitForHDDInterrupt(bool primary) {
  const uint64 start_tick_count = kGetTickCount();
  while (kGetTickCount() - start_tick_count <= HDD_WAIT_TIME) {
    if (primary && hdd_manager.primary_interrupt_occur) {
      return true;
    } else if (!primary && hdd_manager.secondary_interrupt_occur) {
      return true;
    }
  }
  return false;
}