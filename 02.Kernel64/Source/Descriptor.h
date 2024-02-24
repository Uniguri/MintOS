#ifndef MINTOS_DESCRIPTOR_H_
#define MINTOS_DESCRIPTOR_H_

#include "Macro.h"
#include "Types.h"

#define GDT_TYPE_CODE (0x0A)
#define GDT_TYPE_DATA (0x02)
// Type for TSS(Task State Segment).
#define GDT_TYPE_TSS (0x09)
// System/User bit.
#define GDT_FLAGS_LOWER_S BIT(4)
// Descriptor Privilege Level 0; Kernel mode.
#define GDT_FLAGS_LOWER_DPL0 (0x00)
// Descriptor Privilege Level 1; Protected mode.
#define GDT_FLAGS_LOWER_DPL1 BIT(5)
// Descriptor Privilege Level 2; Protected mode.
#define GDT_FLAGS_LOWER_DPL2 BIT(6)
// Descriptor Privilege Level 3; User mode.
#define GDT_FLAGS_LOWER_DPL3 (BIT(6) | BIT(5))
// Present bit.
#define GDT_FLAGS_LOWER_P BIT(7)
// Long mode bit.
#define GDT_FLAGS_UPPER_L BIT(5)
// Default Operation Size bit.
#define GDT_FLAGS_UPPER_DB BIT(6)
// Granularity bit.
#define GDT_FLAGS_UPPER_G BIT(7)

// Default setting for kernel mode code; (CODE, S|DPL0|P).
#define GDT_FLAGS_LOWER_KERNEL_CODE \
  (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
// Default setting for kernel mode data; (DATA, S|DPL0|P).
#define GDT_FLAGS_LOWER_KERNEL_DATA \
  (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
// Default setting for TSS(Task State Segment); (DPL0|P).
#define GDT_FLAGS_LOWER_TSS (GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
// Default setting for user mode code; (CODE, S|DPL3|P).
#define GDT_FLAGS_LOWER_USER_CODE \
  (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P)
// Default setting for user mode data; (DATA, S|DPL3|P).
#define GDT_FLAGS_LOWER_USER_DATA \
  (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P)

// Default setting for code; (G|L).
#define GDT_FLAGS_UPPER_CODE (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
// Default setting for data; (G|L).
#define GDT_FLAGS_UPPER_DATA (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
// Default setting for TSS(Task State Segment); (G).
#define GDT_FLAGS_UPPER_TSS (GDT_FLAGS_UPPER_G)

#define GDT_KERNEL_CODE_SEGMENT (0x08 * 1)
#define GDT_KERNEL_DATA_SEGMENT (0x08 * 2)
#define GDT_TSS_SEGMENT (0x08 * 3)

// Start address of GDTR(Global Descriptor Table Register)
#define GDTR_START_ADDRESS (0x142000)
// The number of 8 uint8s entry
#define GDT_MAX_ENTRY8_COUNT (3)
// The number of 16 uint8s entry and TSS
#define GDT_MAX_ENTRY16_COUNT (1)
// The size of GDT(Global Descriptor Table) table
#define GDT_TABLE_SIZE                        \
  (sizeof(GDTEntry8) * GDT_MAX_ENTRY8_COUNT + \
   sizeof(GDTEntry16) * GDT_MAX_ENTRY16_COUNT)
#define TSS_SEGMENT_SIZE (sizeof(TSSSegment))

// Type of IDT(Interrupt Descriptor Table); Interrupt.
// When type is interrupt, interrupt cannot be occured while handling other
// intterupt.
#define IDT_TYPE_INTERRUPT (0x0E)
// Type of IDT(Interrupt Descriptor Table); Trap.
// When type is interrupt, interrupt can be occured while handling other
// intterupt.
#define IDT_TYPE_TRAP (0x0F)
// Descriptor Privilege Level 0; Kernel mode
#define IDT_FLAGS_DPL0 (0x00)
// Descriptor Privilege Level 1; Protected mode
#define IDT_FLAGS_DPL1 BIT(5)
// Descriptor Privilege Level 2; Protected mode
#define IDT_FLAGS_DPL2 BIT(6)
// Descriptor Privilege Level 3; User mode
#define IDT_FLAGS_DPL3 (BIT(6) | BIT(5))
// Present bit
#define IDT_FLAGS_P BIT(7)
// Interrupt Stack Table 0
#define IDT_FLAGS_IST0 (0)
// Interrupt Stack Table 1
#define IDT_FLAGS_IST1 (1)

// Default setting for IDT(Interrupt Descriptor Table); (DPL0|P).
#define IDT_FLAGS_KERNEL (IDT_FLAGS_DPL0 | IDT_FLAGS_P)
// Default setting for IDT(Interrupt Descriptor Table); (DPL3|P).
#define IDT_FLAGS_USER (IDT_FLAGS_DPL3 | IDT_FLAGS_P)

// The number of IDX entry.
#define IDT_MAX_ENTRY_COUNT (0x64)
// Start address of IDTR(Interrupt Descriptor Table Register).
#define IDTR_START_ADDRESS \
  (GDTR_START_ADDRESS + sizeof(GDTR) + GDT_TABLE_SIZE + TSS_SEGMENT_SIZE)
// Start address of IDT(Interrupt Descriptor Table).
#define IDT_START_ADDRESS (IDTR_STARTADDRESS + sizeof(IDTR))
// The size of IDT(Interrupt Descriptor Table)
#define IDT_TABLE_SIZE (IDT_MAX_ENTRY_COUNT * sizeof(IDTEntry))

// Start address of IST(Interrupt Stack Table)
#define IST_START_ADDRESS (0x700000)
// Size of IST(Interrupt Stack Table)
#define IST_SIZE (0x100000)

#pragma pack(push, 1)
// Basic structure for GDTR and IDTR.
typedef struct kGDTRStruct {
  uint16 limit;
  uint64 base_addr;
  uint8 padding[16 - 2 - 8];
} kGDTRStruct;
_Static_assert(sizeof(kGDTRStruct) == 0x10);
// Structure for GDTR(Global Desciptor Table Register).
// FYI: https://github.com/cacalabs/libcaca/blob/main/kernel/boot/gdt.c#L37
typedef kGDTRStruct GDTR;
// Structure for IDTR(Interrupt Desciptor Table Register).
// FYI: https://github.com/cacalabs/libcaca/blob/main/kernel/boot/idt.c#L43
typedef kGDTRStruct IDTR;

// Structure for GDT(Global Descriptor Table) with 8 bytes.
// FYI: https://github.com/cacalabs/libcaca/blob/main/kernel/boot/gdt.c#L25
typedef struct kGDTEntry8Struct {
  uint16 lower_limit;
  uint16 lower_base_addr;
  uint8 upper_base_addr1;
  uint8 type_and_lower_flag;
  uint8 upper_limit_and_upper_flag;
  uint8 upper_base_addr2;
} GDTEntry8;
_Static_assert(sizeof(GDTEntry8) == 0x08);

// Structure for GDT(Global Descriptor Table) with 16 bytes.
// FYI: https://wiki.osdev.org/Global_Descriptor_Table#Segment_Descriptor
typedef struct kGDTEntry16Struct {
  uint16 lower_limit;
  uint16 lower_base_addr;
  uint8 middle_base_addr1;
  uint8 type_and_lower_flag;
  uint8 upper_limit_and_upper_flag;
  uint8 middle_base_addr2;
  uint32 upper_base_addr;
  uint32 reserved;
} GDTEntry16;
_Static_assert(sizeof(GDTEntry16) == 0x10);

// Structure for TSS(Task State Segment).
// FYI: https://wiki.osdev.org/Task_State_Segment#Long_Mode
typedef struct kTSSDataStruct {
  uint32 reserved1;
  uint64 rsp[3];
  uint64 reserved2;
  uint64 ist[7];
  uint64 reserved3;
  uint16 reserved;
  uint16 io_map_base_addr;
} TSSSegment;
_Static_assert(sizeof(TSSSegment) == 0x68);

// Structure for IDT(Interrupt Descriptor Table).
// FYI: https://wiki.osdev.org/Interrupt_Descriptor_Table#Gate_Descriptor_2
typedef struct kIDTEntryStruct {
  uint16 lower_base_addr;
  uint16 segment_selector;
  uint8 ist;
  uint8 type_and_flag;
  uint16 middle_base_addr;
  uint32 upper_base_addr;
  uint32 reserved;
} IDTEntry;
_Static_assert(sizeof(IDTEntry) == 0x10);
#pragma pack(pop)

void kInitializeGDTTableAndTSS(void);
void kInitializeTSSSegment(TSSSegment* tss);
void kInitializeIDTTables(void);

void kLoadGDTR(uint64 gdtr_addr);
void kLoadTR(uint16 tss_segment_offset);
void kLoadIDTR(uint64 idtr_addr);

void kSetGDTEntry8(GDTEntry8* entry, uint32 base_addr, uint32 limit,
                   uint8 upper_flag, uint8 lower_flag, uint8 type);
void kSetGDTEntry16(GDTEntry16* entry, uint64 base_addr, uint32 limit,
                    uint8 upper_flag, uint8 lower_flag, uint8 type);
void kSetIDTEntry(IDTEntry* entry, void* handler, uint16 selector, uint8 ist,
                  uint8 flag, uint8 type);

void kDummyHandler(void);

#endif