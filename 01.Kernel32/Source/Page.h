#ifndef MINTOS_PAGE_H_
#define MINTOS_PAGE_H_

#include "Types.h"

// Present. If not set, it cannot be accessed.
// It can be used for PML4Entry, PDPTEntry, PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_P (1 << 0)
// Read/Write. If not set, it only allow read.
// It can be used for PML4Entry, PDPTEntry, PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_RW (1 << 1)
// User/Supervisor. If set, it means user level.
// It can be used for PML4Entry, PDPTEntry, PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_US (1 << 2)
// Page Level Write-through.
// If set, It means write-through, else write-back.
// It can be used for PML4Entry, PDPTEntry, PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_PWT (1 << 3)
// Page Level Cache Disable. If set, it means disable cache.
// It can be used for PML4Entry, PDPTEntry, PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_PCD (1 << 4)
// Accessed. If set, it means this entry accessed.
// It can be used for PML4Entry, PDPTEntry, PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_A (1 << 5)
// Dirty. If set, it means this entry modified.
// It can be used for PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_D (1 << 6)
// Page Size. If not set, page size is 4KB.
// If set, page size is 2MB when PAE is set on CR4 or 4MB when PAE is not.
// It can be used for PDEntry.
#define PAGE_LOWER_FLAGS_PS (1 << 7)
// Global bits.
// It can be used for PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_G (1 << 8)
// Page Attribute Table Index.
// It can be used for PDEntry, PTEntry.
#define PAGE_LOWER_FLAGS_PAT (1 << 12)
// Executable Bits. if set it means No-Execute.
// It can be used for PML4Entry, PDPTEntry, PDEntry, PTEntry.
#define PAGE_UPPER_FLAGS_EXB (1 << 31)
// Default (P | RW) Page Flags.
#define PAGE_LOWER_FLAGS_DEFAULT (PAGE_LOWER_FLAGS_P | PAGE_LOWER_FLAGS_RW)

// Size of page table; 4KB
#define PAGE_TABLE_SIZE 0x1000
#define PAGE_MAX_ENTRY_COUNT 0x200
// Size of page; 2MB
#define PAGE_DEFAULT_SIZE 0x200000

// A global structure for PML4Entry, PDPTEntry, PDEntry, PTEntry.
typedef struct GlobalPageTableEntryStruct {
  uint32 attribute_and_lower_base_addr;
  uint32 upper_base_addr_and_exb;
} GlobalPageTableEntryStruct;
_Static_assert(sizeof(GlobalPageTableEntryStruct) == 8,
               "GlobalPageTableEntryStruct struct size != 8");
// PML4 Entry Structure. They locate on 0x100000.
// This has two member: attribute_and_lower_base_addr, upper_base_addr_and_exb.
// FYI: https://gist.github.com/mvankuipers/94e4f794c6909bd124d7eaf6e840a232.
typedef GlobalPageTableEntryStruct PML4Entry;
// Page Directory Pointer Table Entry. They locate on 0x101000.
// This has two member: attribute_and_lower_base_addr, upper_base_addr_and_exb.
// FYI: https://gist.github.com/mvankuipers/c88cdc2a2b3b5ca9d97820b85b33f11c.
typedef GlobalPageTableEntryStruct PDPTEntry;
// Page Directory Entry. They locate on 0x102000.
// This has two member: attribute_and_lower_base_addr, upper_base_addr_and_exb.
// FYI: https://gist.github.com/mvankuipers/5393a5b8c0a9872385a9cccad53d2e43.
typedef GlobalPageTableEntryStruct PDEntry;
// Page Table Entry.
// This has two member: attribute_and_lower_base_addr, upper_base_addr_and_exb.
// FYI: https://gist.github.com/mvankuipers/49549620ee76250fc29642bde95a33e1.
typedef GlobalPageTableEntryStruct PTEntry;

// Set data of page table entry
//@param entry: page table entry pointer
//@param upper_base_addr: upper 32bits of base addr; (0 <= addr <= 0xFF)
//@param lower_base_addr: lower 32bits of base addr
//@param lower_flags: lower flags ( P, RW, US, PWT, PCD, A, D, PS, G, PAT)
//@param upper_flags: upper flags (EXB)
void kSetPageEntryData(PTEntry* entry, uint32 upper_base_addr,
                       uint32 lower_base_addr, uint32 lower_flags,
                       uint32 upper_flags);

// Initialize Page Tables
void kInitializePageTables(void);

#endif