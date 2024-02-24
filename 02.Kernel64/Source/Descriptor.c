#include "Descriptor.h"

#include "Utility.h"

void kInitializeGDTTableAndTSS(void) {
  GDTR* gdtr = (GDTR*)GDTR_START_ADDRESS;
  GDTEntry8* entry = (GDTEntry8*)(GDTR_START_ADDRESS + sizeof(GDTR));
  gdtr->limit = GDT_TABLE_SIZE - 1;
  gdtr->base_addr = (uint64)entry;
  TSSSegment* tss = (TSSSegment*)((uint64)entry + GDT_TABLE_SIZE);
  kSetGDTEntry8(&entry[0], 0, 0, 0, 0, 0);
  kSetGDTEntry8(&entry[1], 0, 0xFFFFF, GDT_FLAGS_UPPER_CODE,
                GDT_FLAGS_LOWER_KERNEL_CODE, GDT_TYPE_CODE);
  kSetGDTEntry8(&entry[2], 0, 0xFFFFF, GDT_FLAGS_UPPER_DATA,
                GDT_FLAGS_LOWER_KERNEL_DATA, GDT_TYPE_DATA);
  kSetGDTEntry16((GDTEntry16*)&entry[3], (uint64)tss, sizeof(TSSSegment) - 1,
                 GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS, GDT_TYPE_TSS);
  kInitializeTSSSegment(tss);
}

void kInitializeTSSSegment(TSSSegment* tss) {
  memset(tss, 0, sizeof(TSSSegment));
  tss->ist[0] = IST_START_ADDRESS + IST_SIZE;
  // Disabvle IO Map by setting IO bigger than limit.
  tss->io_map_base_addr = 0xFFFF;
}

void kInitializeIDTTables(void) {
  IDTR* idtr = (IDTR*)IDTR_START_ADDRESS;
  IDTEntry* entry = (IDTEntry*)(IDTR_START_ADDRESS + sizeof(IDTR));
  idtr->base_addr = (uint64)entry;
  idtr->limit = IDT_TABLE_SIZE - 1;
  for (int i = 0; i < IDT_MAX_ENTRY_COUNT; ++i) {
    kSetIDTEntry(&entry[i], kDummyHandler, 0x08, IDT_FLAGS_IST1,
                 IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
  }
}

void kLoadGDTR(uint64 gdtr_addr) {
  asm volatile("lgdt [%[gdtr_addr]]" : : [gdtr_addr] "r"(gdtr_addr));
}
void kLoadTR(uint16 tss_segment_offset) {
  asm volatile("ltr %[tss_segment_offset]"
               :
               : [tss_segment_offset] "r"(tss_segment_offset));
}
void kLoadIDTR(uint64 idtr_addr) {
  asm volatile("lidt [%[idtr_addr]]" : : [idtr_addr] "r"(idtr_addr));
}

void kSetGDTEntry8(GDTEntry8* entry, uint32 base_addr, uint32 limit,
                   uint8 upper_flag, uint8 lower_flag, uint8 type) {
  entry->lower_limit = limit & 0xFFFF;
  entry->lower_base_addr = base_addr & 0xFFFF;
  entry->upper_base_addr1 = (base_addr >> 16) & 0xFF;
  entry->type_and_lower_flag = (lower_flag | type);
  entry->upper_limit_and_upper_flag = (((limit >> 16) & 0xFF) | upper_flag);
  entry->upper_base_addr2 = (base_addr >> 24) & 0xFF;
}

void kSetGDTEntry16(GDTEntry16* entry, uint64 base_addr, uint32 limit,
                    uint8 upper_flag, uint8 lower_flag, uint8 type) {
  entry->lower_limit = limit & 0xFFFF;
  entry->lower_base_addr = base_addr & 0xFFFF;
  entry->middle_base_addr1 = (base_addr >> 16) & 0xFF;
  entry->type_and_lower_flag = (lower_flag | type);
  entry->upper_limit_and_upper_flag = (((limit >> 16) & 0xFF) | upper_flag);
  entry->middle_base_addr2 = (base_addr >> 24) & 0xFF;
  entry->upper_base_addr = base_addr >> 32;
  entry->reserved = 0;
}

void kSetIDTEntry(IDTEntry* entry, void* handler, uint16 selector, uint8 ist,
                  uint8 flag, uint8 type) {
  entry->lower_base_addr = (uint64)handler & 0xFFFF;
  entry->segment_selector = selector;
  entry->ist = ist & 0x3;
  entry->type_and_flag = type | flag;
  entry->middle_base_addr = ((uint64)handler >> 16) & 0xFFFF;
  entry->upper_base_addr = (uint64)handler >> 32;
  entry->reserved = 0;
}

void kDummyHandler(void) {
  kPrintString(0, 0, "====================================================");
  kPrintString(0, 1, "          Dummy Interrupt Handler Execute~!!!       ");
  kPrintString(0, 2, "           Interrupt or Exception Occur~!!!!        ");
  kPrintString(0, 3, "====================================================");

  while (1)
    ;
}