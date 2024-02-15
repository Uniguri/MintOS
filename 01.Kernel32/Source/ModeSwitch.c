#include "ModeSwitch.h"

void kReadCPUID(uint32 in_eax, uint32* eax, uint32* ebx, uint32* ecx,
                uint32* edx) {
  // registers to store output of CPUID.
  uint32 out_eax = 0, out_ebx = 0, out_ecx = 0, out_edx = 0;

  // We can use pushad & popad
  // because we are in x86-32 when this function called.
  asm("pushad\n"
      "mov eax, %[in_eax]\n"
      "cpuid\n"
      "mov [%[out_eax]], eax\n"
      "mov [%[out_ebx]], ebx\n"
      "mov [%[out_ecx]], ecx\n"
      "mov [%[out_edx]], edx\n"
      "popad\n"
      : [out_eax] "=m"(out_eax), [out_ebx] "=m"(out_ebx),
        [out_ecx] "=m"(out_ecx), [out_edx] "=m"(out_edx)
      : [in_eax] "r"(in_eax));

  *eax = out_eax;
  *ebx = out_ebx;
  *ecx = out_ecx;
  *edx = out_edx;
}