#include "ModeSwitch.h"
#include "Page.h"
#include "Types.h"

// Print string on (x, y).
// This function does not change attrubutes.
// @param x: 0 means left. (0 <= x < 80)
// @param y: 0 means top. (0 <= y < 25)
void kPrintString(const int x, const int y, const char* string);

// Set [0x100000, 0x600000) of memory to zeros.
// @return false when fail to set.
bool kInitializeKernel64Area(void);

// Copy kernel64(IA-32e mode Kernel) image to 0x200000(2MB)
void kCopyKernel64ImageTo2MB(void);

void Main() {
#define MAKE_STUCK(x) while (x)
  kPrintString(0, 3, "Protected Mode C Language Kernel Start......[Pass]");

  // Initialize memory([0x100000, 0x600000)) for IA-32e mode Kernel.
  kPrintString(0, 4, "IA-32e Kernel Area Initialize...............[    ]");
  if (!kInitializeKernel64Area()) {
    kPrintString(45, 4, "Fail");
    MAKE_STUCK(1);
  }
  kPrintString(45, 4, "Pass");

  // Create page tables for IA-32e mode Kernel.
  kPrintString(0, 5, "IA-32e Page Tables Initialize...............[    ]");
  kInitializePageTables();
  kPrintString(45, 5, "Pass");

  {
    // registers to store output of CPUID.
    uint32 eax = 0, ebx = 0, ecx = 0, edx = 0;
    char cpu_vendor_name[13] = {
        0,
    };

    // Read CPU vendor
    kReadCPUID(0, &eax, &ebx, &ecx, &edx);
    *((uint32*)cpu_vendor_name + 0) = ebx;
    *((uint32*)cpu_vendor_name + 1) = edx;
    *((uint32*)cpu_vendor_name + 2) = ecx;
    kPrintString(0, 6,
                 "Processor Vendor String.....................[            ]");
    kPrintString(45, 6, cpu_vendor_name);

    // Check for 64-bit support
    kReadCPUID(0x80000001, &eax, &ebx, &ecx, &edx);
    kPrintString(0, 7, "64bit Mode Support Check....................[    ]");
    if (!(edx & (1 << 29))) {
      kPrintString(45, 7, "Fail");
      MAKE_STUCK(1);
    }
    kPrintString(45, 7, "Pass");
  }

  // Copy kernel64(IA-32e mode Kernel) image to 0x200000(2MB).
  kPrintString(0, 8, "Copy IA-32e Kernel To 2M Address............[    ]");
  kCopyKernel64ImageTo2MB();
  kPrintString(45, 8, "Pass");

  // Switch mode to IA-32e
  kPrintString(0, 9, "Switch To IA-32e Mode");
  kSwitchAndExecute64bitKernel();

  MAKE_STUCK(1);
#undef MAKE_STUCK
}

void kPrintString(const int x, const int y, const char* string) {
  Character* screen = (Character*)0xB8000 + 80 * y + x;
  for (const char* p = string; *p; ++p) {
    screen->charactor = *p;
    ++screen;
  }
}

bool kInitializeKernel64Area(void) {
  for (uint32* current_address = (uint32*)0x100000;
       current_address < (uint32*)0x600000; ++current_address) {
    *current_address = 0;
    // If *current_address is not zero, an error occurs and return false.
    if (*current_address) {
      return false;
    }
  }

  return true;
}

void kCopyKernel64ImageTo2MB(void) {
  const uint32 kSectorSize = 0x200;

  // BootLoader addr=0x7C00, TOTAL_SELECTOR_COUNT offset=0x02
  const uint16 kTotalSectorCount = *(uint16*)(0x7C00 + 0x02);
  // BootLoader addr=0x7C00, KERNEL32_SECTOR_COUNT offset=0x04
  const uint16 kKernel32SectorCount = *(uint16*)(0x7C00 + 0x04);
  // kKernel32SectorCount + kKernel64SectorCount = kTotalSectorCount
  const uint16 kKernel64SectorCount = kTotalSectorCount - kKernel32SectorCount;

  // Kernel images are loaded on 0x10000.
  // So, Kernel64 image is loaded on 0x10000+kKernel32SectorCount*kSectorSize.
  uint32* src_addr = (uint32*)(0x10000 + kKernel32SectorCount * kSectorSize);
  // Kernel64 image is loaded on 0x200000(2MB).
  uint32* dst_addr = (uint32*)(0x200000);
  for (int i = 0; i < kKernel64SectorCount; ++i) {
    for (int j = 0; j < 512 / 4; ++j) {
      *(dst_addr++) = *(src_addr++);
    }
  }
}