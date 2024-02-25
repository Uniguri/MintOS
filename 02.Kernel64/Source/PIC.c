#include "PIC.h"

#include "HardwarePort.h"
#include "Macro.h"

void kInitializePIC(void) {
  // ICW1(0x20), ICW1_INIT | ICW1_ICW4
  kSetPortByte(PIC_MASTER_CMD_PORT, BIT(4) | BIT(0));
  // ICW2(0x21), Interrupt vector(0x20)
  kSetPortByte(PIC_MASTER_DATA_PORT, PIC_IRQ_START_VECTOR);
  // ICW3(0x21), PIC slave location in bit
  kSetPortByte(PIC_MASTER_DATA_PORT, BIT(2));
  // ICW4(0x21), uPM
  kSetPortByte(PIC_MASTER_DATA_PORT, BIT(0));

  // ICW1(0xA0), ICW1_INIT | ICW_ICW4
  kSetPortByte(PIC_SLAVE_CMD_PORT, BIT(4) | BIT(0));
  // ICW2(0xA1), Interrupt vector(0x20 + 8)
  kSetPortByte(PIC_SLAVE_DATA_PORT, PIC_IRQ_START_VECTOR + 8);
  // ICW3(0xA1), PIC master location in integer
  kSetPortByte(PIC_SLAVE_DATA_PORT, 0x02);
  // ICW4(0xA1), uPM
  kSetPortByte(PIC_SLAVE_DATA_PORT, BIT(0));
}

void kMaskPICInterrupt(uint16 irq_bit_mask) {
  kSetPortByte(PIC_MASTER_DATA_PORT, (uint8)irq_bit_mask);
  kSetPortByte(PIC_SLAVE_DATA_PORT, (uint8)(irq_bit_mask >> 8));
}

void kSendEOIToPIC(int irq_number) {
  // OCW2(0x20), EOI
  kSetPortByte(PIC_MASTER_CMD_PORT, BIT(5));

  // If irq_number is bigger than or equal to 8, it is slave.
  if (irq_number >= 8) {
    // OCW2(0xA0), EOI
    kSetPortByte(PIC_SLAVE_CMD_PORT, BIT(5));
  }
}