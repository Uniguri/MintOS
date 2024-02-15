#include "Page.h"

#include "Types.h"

void kSetPageEntryData(PTEntry* entry, uint32 upper_base_addr,
                       uint32 lower_base_addr, uint32 lower_flags,
                       uint32 upper_flags) {
  entry->attribute_and_lower_base_addr = lower_base_addr | lower_flags;
  entry->upper_base_addr_and_exb = (upper_base_addr & 0xFF) | upper_flags;
}

void kInitializePageTables(void) {
  {
    // Set PML4Entries to zeros except first one.
    // The first PML4Entry: base = 0x101000, flags = (P|RW).
    PML4Entry* pml4_entries = (PML4Entry*)0x100000;
    kSetPageEntryData(&pml4_entries[0], 0, 0x101000, PAGE_LOWER_FLAGS_DEFAULT,
                      0);
    for (int i = 1; i < PAGE_MAX_ENTRY_COUNT; ++i) {
      kSetPageEntryData(&pml4_entries[i], 0, 0, 0, 0);
    }
  }
  {
    // First 64th PDPTEntry: base = 0x102000+i*PAGE_TABLE_SIZE, flags = (P|RW).
    // set other PDPTEntries to zeros.
    PDPTEntry* pdpt_entries = (PDPTEntry*)0x101000;
    for (int i = 0; i < 64; ++i) {
      kSetPageEntryData(&pdpt_entries[i], 0, 0x102000 + i * PAGE_TABLE_SIZE,
                        PAGE_LOWER_FLAGS_DEFAULT, 0);
    }
    for (int i = 64; i < PAGE_MAX_ENTRY_COUNT; ++i) {
      kSetPageEntryData(&pdpt_entries[i], 0, 0, 0, 0);
    }
  }
  {
    // Mapping PDs on PAGE_DEFAULT_SIZE(0x200000).
    // PDEntry[i]: base = i*0x200000, flags = (P|RW|PS).
    // So their size is 2MB.
    PDEntry* pd_entry = (PDEntry*)0x102000;
    uint64 mapping_addr = 0;
    for (int i = 0; i < PAGE_MAX_ENTRY_COUNT * 0x40; ++i) {
      const uint64 upper_mapping_addr = mapping_addr >> 32;
      const uint64 lower_mapping_addr = mapping_addr & 0xFFFFFFFF;
      kSetPageEntryData(&pd_entry[i], upper_mapping_addr, lower_mapping_addr,
                        PAGE_LOWER_FLAGS_DEFAULT | PAGE_LOWER_FLAGS_PS, 0);
      mapping_addr += PAGE_DEFAULT_SIZE;
    }
  }
}