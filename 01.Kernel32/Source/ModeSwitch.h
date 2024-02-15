#ifndef MINTOS_MODESWITCH_H
#define MINTOS_MODESWITCH_H

#include "Types.h"

// Read CPU Identification using CPUID instruction.
// Implemented in ./ModeSwitch.c.
// Check out: https://en.wikipedia.org/wiki/CPUID#Calling_CPUID
//@param in_eax: EAX value used calling CPUID; It must be 0 or 0x80000000
//@param eax: pointer to store output (eax).
//@param ebx: pointer to store output (ebx).
//@param ecx: pointer to store output (ecx).
//@param edx: pointer to store output (edx).
void kReadCPUID(uint32 in_eax, uint32* eax, uint32* ebx, uint32* ecx,
                uint32* edx);

// Switch system mode to IA-32e and Execute 64 bits kernel.
// Implemented in ./ModeSwitchASM.asm.
// Check out https://wiki.osdev.org/Setting_Up_Long_Mode.
void kSwitchAndExecute64bitKernel(void);

#endif