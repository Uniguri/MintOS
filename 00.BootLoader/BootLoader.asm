[ORG 0x7C00]
[BITS 16]

SECTION .text

jmp START

; TOTAL_SELECTOR_COUNT: dw 0x0400
TOTAL_SELECTOR_COUNT: dw 1

START:
    mov ax, 0xB800
    mov es, ax

    ; Create stack with 64KB on 0x0000:0000-0x0000:FFFF
    mov ax, 0x0000
    mov ss, ax
    mov sp, 0xFFFE
    mov bp, 0xFFFE

    ; Clear screen
    mov si, 0
.SCREEN_CLEAR_LOOP:
    mov byte [es:si], 0
    mov byte [es:si+1], 0x0A
    add si, 2
    cmp si, 80*25*2
    jl .SCREEN_CLEAR_LOOP

    ; Print starting message
    push MESSAGE1
    push 0
    push 0
    call PRINT_MESSAGE
    add sp, 6

    ; Print OS image loading message
    push IMAGE_LOADING_MESSAGE
    push 1
    push 0
    call PRINT_MESSAGE
    add sp, 6

    ; Load OS image
    ;; Reset disk before loading
RESET_DISK:
    mov ax, 0
    mov dl, 0
    int 0x13
    jc HANDLE_DISK_ERROR

    ;; Read sector on disk
    mov si, 0x1000
    mov es, si
    mov bx, 0x0000

    mov di, word [TOTAL_SELECTOR_COUNT]

READ_DATA:
    cmp di, 0
    je READ_END
    sub di, 0x01

    mov ah, 0x02
    mov al, 0x01
    mov ch, byte [TRACK_NUMBER]
    mov cl, byte [SECTOR_NUMBER]
    mov dh, byte [HEAD_NUMBER]
    mov dl, 0x00
    int 0x13 ; Call BIOD read function

    add si, 0x0020
    mov es, si

    mov al, byte [SECTOR_NUMBER]
    add al, 0x01
    mov byte [SECTOR_NUMBER], al
    cmp al, 37
    jl READ_DATA

    xor byte [HEAD_NUMBER], 0x01
    mov byte [SECTOR_NUMBER], 0x01

    cmp byte [HEAD_NUMBER], 0x00
    jne READ_DATA

    add byte [TRACK_NUMBER], 0x01
    jmp READ_DATA

READ_END:
    push LOADING_COMPLETE_MESSAGE
    push 1
    push 20
    call PRINT_MESSAGE
    add sp, 6

    jmp 0x1000:0x0000

; Functions
HANDLE_DISK_ERROR:
    push DISK_ERROR_MESSAGE
    push 1
    push 20
    call PRINT_MESSAGE
    jmp $

PRINT_MESSAGE:
    push bp
    mov bp, sp

    push es
    push si
    push di
    push ax
    push cx
    push dx

    mov ax, 0xB800
    mov es, ax

    mov ax, word [bp + 6]
    mov si, 160
    mul si
    mov di, ax

    mov ax, word [bp + 4]
    mov si, 2
    mul si
    add di, ax

    mov si, word [bp + 8]

.MESSAGE_LOOP:
    mov cl, byte [si]
    cmp cl, 0
    je .MESSAGE_END

    mov byte [es:di], cl
    add si, 1
    add di, 2
    jmp .MESSAGE_LOOP

.MESSAGE_END:
    pop dx
    pop cx
    pop ax
    pop di
    pop si
    pop es
    pop bp
    ret

; DATA SECTION
MESSAGE1: db 'MINT64 OS Boot Loader Start', 0
DISK_ERROR_MESSAGE: db 'DISK Error', 0
IMAGE_LOADING_MESSAGE: db 'OS Image Loading...', 0
LOADING_COMPLETE_MESSAGE: db 'Complete Loading', 0

SECTOR_NUMBER: db 0x02
HEAD_NUMBER: db 0x00
TRACK_NUMBER: db 0x00

times 510 - ($ - $$) db 0x00

dw 0xAA55