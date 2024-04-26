#ifndef MINTOS_HARDDISK_H_
#define MINTOS_HARDDISK_H_

#include "Synchronization.h"
#include "Types.h"

#define HDD_PORT_PRIMARY_BASE (0x1F0)
#define HDD_PORT_SECONDARY_BASE (0x170)

#define HDD_PORT_INDEX_DATA (0x00)
#define HDD_PORT_INDEX_SECTOR_COUNT (0x02)
#define HDD_PORT_INDEX_SECTOR_NUMBER (0x03)
#define HDD_PORT_INDEX_CYLINDER_LSB (0x04)
#define HDD_PORT_INDEX_CYLINDER_MSB (0x05)
#define HDD_PORT_INDEX_DRIVE_AND_HEAD (0x06)
#define HDD_PORT_INDEX_STATUS (0x07)
#define HDD_PORT_INDEX_COMMAND (0x07)
#define HDD_PORT_INDEX_DEVICE_CONTROL (0x206)

#define HDD_COMMAND_READ (0x20)
#define HDD_COMMAND_WRITE (0x30)
#define HDD_COMMAND_INDENTIFY (0xEC)

#define HDD_STATUS_ERROR (0x01)
#define HDD_STATUS_INDEX (0x02)
#define HDD_STATUS_CORRECTED_DATA (0x04)
#define HDD_STATUS_DATA_REQUEST (0x08)
#define HDD_STATUS_SEEK_COMPLETE (0x10)
#define HDD_STATUS_WRITE_FAULT (0x20)
#define HDD_STATUS_READY (0x40)
#define HDD_STATUS_BUSY (0x80)

#define HDD_DRIVE_AND_HEAD_LBA (0xE0)
#define HDD_DRIVE_AND_HEAD_SLAVE (0x10)

#define HDD_WAIT_TIME (500)
#define HDD_MAX_BULK_SECTOR_COUNT (256)

typedef struct kHDDInformationStruct {
  uint16 configuration;

  uint16 number_of_cylinder;
  uint16 reserved1;

  uint16 number_of_head;
  uint16 unformatted_bytes_per_track;
  uint16 unformatted_bytes_per_sector;

  uint16 number_of_sector_per_cylinder;
  uint16 inter_sector_gap;
  uint16 bytes_in_phase_lock;
  uint16 number_of_vendor_unique_status_word;

  uint16 serial_number[10];
  uint16 controller_type;
  uint16 buffer_size;

  uint16 number_of_ecc_bytes;
  uint16 firmware_revision[4];

  uint16 model_number[20];
  uint16 reserved2[13];

  uint16 total_sectors;
  uint16 reserved3[196];
} HDDInformation;

typedef struct kHDDManagerStruct {
  bool hdd_detected;
  bool can_write;

  volatile bool primary_interrupt_occur;
  volatile bool secondary_interrupt_occur;

  Mutex mutex;

  HDDInformation hdd_information;
} HddManager;

// Initialize HDD Manager.
bool kInitializeHDD(void);

// Set HDD Manager's interrupt flag.
// @param primary: is primary dist?
// @param flag: flag to set.
void kSetHDDInterruptFlag(bool primary, bool flag);

// Read HDD Information from HDD.
// @param primary: Do read info from primary disk?
// @param master: Do read info from master controller?
// @param hdd_info: pointer to store information.
// @return true if HDD is detected and can read info from it.
bool kReadHDDInformation(bool primary, bool master, HDDInformation* hdd_info);

// Read data from HDD sectors.
// @return written sector count.
int kReadHDDSector(bool primary, bool master, uint32 lba, int sector_count,
                   char* buffer);

// Write data to HDD sectors.
// @return written sector count.
int kWriteHDDSector(bool primary, bool master, uint32 lba, int sector_count,
                    const char* buffer);

static uint8 kReadHDDStatus(bool primary);
static void kSwapByteInWord(uint16* data, int word_count);
static bool kWaitForHDDReady(bool primary);
static bool kWaitForHDDNoBusy(bool primary);
static bool kWaitForHDDInterrupt(bool primary);

#endif