[BITS 64]

SECTION .text

; void kCommonExceptionHandler(int vector_number, uint64 error_code)
extern kCommonExceptionHandler
; void kCommonInterruptHandler(int vector_number)
extern kCommonInterruptHandler
; void kKeyboardHandler(int vector_number)
extern kKeyboardHandler
; void kTimerHandler(int vector_number)
extern kTimerHandler

; ISR(Interrupt Service Routin) for Exception.
global kISRDivideError, kISRDebug, kISRNMI, kISRBreakPoint, kISROverflow
global kISRBoundRangeExceeded, kISRInvalidOpcode, kISRDeviceNotAvailable, kISRDoubleFault
global kISRCoprocessorSegmentOverrun, kISRInvalidTSS, kISRSegmentNotPresent
global kISRStackSegmentFault, kISRGeneralProtection, kISRPageFault, kISR15
global kISRFPUError, kISRAlignmentCheck, kISRMachineCheck, kISRSIMDError, kISRETCException

; ISR(Interrupt Service Routin) for Interrupt.
global kISRTimer, kISRKeyboard, kISRSlavePIC, kISRSerial2, kISRSerial1, kISRParallel2
global kISRFloppy, kISRParallel1, kISRRTC, kISRReserved, kISRNotUsed1, kISRNotUsed2
global kISRMouse, kISRCoprocessor, kISRHDD1, kISRHDD2, kISRETCInterrupt

%macro SAVE_CONTEXT 0
  push rbp
  mov rbp, rsp
  push rax
  push rbx
  push rcx
  push rdx
  push rdi
  push rsi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15

  mov ax, ds
  push rax
  mov ax, es
  push rax
  push fs
  push gs

  mov ax, 0x10 ; Kernel Data Segment Descriptor
  mov ds, ax
  mov es, ax
  mov gs, ax
  mov fs, ax
%endmacro

%macro LOAD_CONTEXT 0
  pop gs
  pop fs
  pop rax
  mov es, ax 
  pop rax
  mov ds, ax

  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rsi
  pop rdi
  pop rdx
  pop rcx
  pop rbx
  pop rax
  pop rbp
%endmacro

; Exception

; #0, Divide Error ISR
kISRDivideError:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 0
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #1, Debug ISR
kISRDebug:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 1
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #2, NMI ISR
kISRNMI:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 2
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #3, BreakPoint ISR
kISRBreakPoint:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 3
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #4, Overflow ISR
kISROverflow:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 4
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #5, Bound Range Exceeded ISR
kISRBoundRangeExceeded:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 5
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #6, Invalid Opcode ISR
kISRInvalidOpcode:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 6
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #7, Device Not Available ISR
kISRDeviceNotAvailable:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 7
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #8, Double Fault ISR
kISRDoubleFault:
    SAVE_CONTEXT

    ; Call handler with exception number and error code.
    mov rdi, 8
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler

    LOAD_CONTEXT
    add rsp, 8
    iretq

; #9, Coprocessor Segment Overrun ISR
kISRCoprocessorSegmentOverrun:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 9
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #10, Invalid TSS ISR
kISRInvalidTSS:
    SAVE_CONTEXT

    ; Call handler with exception number and error code.
    mov rdi, 10
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler

    LOAD_CONTEXT
    add rsp, 8
    iretq

; #11, Segment Not Present ISR
kISRSegmentNotPresent:
    SAVE_CONTEXT

    ; Call handler with exception number and error code.
    mov rdi, 11
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler

    LOAD_CONTEXT
    add rsp, 8
    iretq

; #12, Stack Segment Fault ISR
kISRStackSegmentFault:
    SAVE_CONTEXT

    ; Call handler with exception number and error code.
    mov rdi, 12
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler

    LOAD_CONTEXT
    add rsp, 8
    iretq

; #13, General Protection ISR
kISRGeneralProtection:
    SAVE_CONTEXT

    ; Call handler with exception number and error code.
    mov rdi, 13
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler

    LOAD_CONTEXT
    add rsp, 8
    iretq

; #14, Page Fault ISR
kISRPageFault:
    SAVE_CONTEXT

    ; Call handler with exception number and error code.
    mov rdi, 14
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler

    LOAD_CONTEXT
    add rsp, 8
    iretq

; #15, Reserved ISR
kISR15:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 15
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #16, FPU Error ISR
kISRFPUError:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 16
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #17, Alignment Check ISR
kISRAlignmentCheck:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 17
    mov rsi, qword [ rbp + 8 ]
    call kCommonExceptionHandler

    LOAD_CONTEXT
    add rsp, 8
    iretq

; #18, Machine Check ISR
kISRMachineCheck:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 18
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #19, SIMD Floating Point Exception ISR
kISRSIMDError:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 19
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq

; #20~#31, Reserved ISR
kISRETCException:
    SAVE_CONTEXT

    ; Call handler with exception number.
    mov rdi, 20
    mov rsi, 0
    call kCommonExceptionHandler

    LOAD_CONTEXT
    iretq


; Interrupt

; #32, Timer ISR
kISRTimer:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 32
    call kTimerHandler

    LOAD_CONTEXT
    iretq

; #33, Keyboard ISR
kISRKeyboard:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 33
    call kKeyboardHandler

    LOAD_CONTEXT
    iretq

; #34, Slave PIC ISR
kISRSlavePIC:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 34
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #35, Serial Port 2 ISR
kISRSerial2:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 35
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #36, Serial Port 1 ISR
kISRSerial1:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 36
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #37, Parallel port 2 ISR
kISRParallel2:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 37
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #38, Floppy Disk Controller ISR
kISRFloppy:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 38
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #39, Parallel port 1 ISR
kISRParallel1:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 39
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #40, RTC ISR
kISRRTC:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 40
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #41, Reserved Interrupt ISR
kISRReserved:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 41
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #42, Not Used
kISRNotUsed1:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 42
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #43, Not Used
kISRNotUsed2:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 43
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #44, Mouse ISR
kISRMouse:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 44
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #45, Coprocessor ISR
kISRCoprocessor:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 45
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #46, Hard Disk 1 ISR
kISRHDD1:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 46
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #47, Hard Disk 2 ISR
kISRHDD2:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 47
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq

; #48 Other Interrupts ISR
kISRETCInterrupt:
    SAVE_CONTEXT

    ; Call handler with interrupt number
    mov rdi, 48
    call kCommonInterruptHandler

    LOAD_CONTEXT
    iretq