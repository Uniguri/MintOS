#ifndef MINTOS_PIC_H_
#define MINTOS_PIC_H_

#include "Types.h"

// Command port of PIC master.
#define PIC_MASTER_CMD_PORT 0x20
// Data port of PIC master.
#define PIC_MASTER_DATA_PORT 0x21
// Command port of PIC slave.
#define PIC_SLAVE_CMD_PORT 0xA0
// Data port of PIC slave.
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_IRQ_START_VECTOR 0x20

// Initialize PIC(Programable Interrupt Controller).
// FYI: https://wiki.osdev.org/8259_PIC#Initialisation
void kInitializePIC(void);
// Mask PIC interrupt.
// @param irq_bit_mask: bit mask for IRQ. IRQ0...IRQ7 is for Master,
// IRQ8...IRQ15 is for Slave.
void kMaskPICInterrupt(uint16 irq_bit_mask);
// Send EOI(End Of Interrupt) to PIC(Programable Interrupt Controller).
// @param irq_number: the number of IRQ(Interrupt Request). (0<=irq_number<16)
void kSendEOIToPIC(int irq_number);

#endif