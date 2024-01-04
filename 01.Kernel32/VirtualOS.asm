[ORG 0x0000]
[BITS 16]

SECTION .text

jmp 0x1000:START

SECTOR_COUNT: dw 0x0000
TOTAL_SELECTOR_COUNT equ 0x400

; Code
START:
    mov ax, cs
    mov ds, ax
    mov ax, 0xB800

    mov es, ax
    ; Create each sector
    %assign i 0
    %rep TOTAL_SELECTOR_COUNT
        %assign i i+1

        mov ax, 2
        mul word [SECTOR_COUNT]
        mov si, ax

        mov byte [es:si + (160*2)], '0' + (i % 10)
        add word [SECTOR_COUNT], 1

        %if i == TOTAL_SELECTOR_COUNT
            jmp $
        %else
            jmp (0x1000 + i * 0x20): 0x0000
        %endif

        times (512 - ($ - $$) % 512) db 0x00
    %endrep