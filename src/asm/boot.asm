[org 0x7c00]
[bits 16]

jmp short start
nop

OEMLabel            db "NTHBOOT "
BytesPerSector      dw 512
SectorsPerCluster   db 1
ReservedForBoot     dw 1
NumberOfFATs        db 2
RootDirEntries      dw 224
LogicalSectors      dw 2880
MediumByte          db 0xF0
SectorsPerFat       dw 9
SectorsPerTrack     dw 18
Sides               dw 2
HiddenSectors       dd 0
LargeSectors        dd 0
DriveNo             dw 0
Signature           db 41
VolumeID            dd 0x00000000
VolumeLabel         db "NTH OS     "
FileSystem          db "FAT12   "

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [DriveNo], dl

    mov si, entry_string
    call print

    mov ah, 0
    int 0x13
    jc disk_error

    mov ax, 19
    mov cx, 14
    mov bx, 0x7E00
    call read_sectors

    mov cx, [RootDirEntries]
    mov di, 0x7E00
.search_loop:
    push cx
    mov cx, 11
    mov si, kernel_filename
    push di
    rep cmpsb
    pop di
    je .found_kernel
    pop cx
    add di, 32
    loop .search_loop
    jmp not_found_error

.found_kernel:
    pop cx
    mov ax, [di + 26]
    mov [kernel_cluster], ax

    sub ax, 2
    add ax, 33
    mov cx, 1
    mov bx, 0x1000
    call read_sectors

    in al, 0x92
    or al, 2
    out 0x92, al

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

read_sectors:
.read_loop:
    push ax
    push cx
    push bx
    call lba_to_chs
    mov di, 3
.retry:
    mov ah, 0x02
    mov al, 1
    mov ch, [absolute_track]
    mov cl, [absolute_sector]
    mov dh, [absolute_head]
    mov dl, [DriveNo]
    int 0x13
    jnc .read_success

    xor ah, ah
    int 0x13
    dec di
    jnz .retry

    jmp disk_error

.read_success:
    pop bx
    add bx, 512
    pop cx
    pop ax
    inc ax
    loop .read_loop
    ret

lba_to_chs:
    xor dx, dx
    div word [SectorsPerTrack]
    inc dl
    mov [absolute_sector], dl
    xor dx, dx
    div word [Sides]
    mov [absolute_head], dl
    mov [absolute_track], al
    ret

disk_error:
    mov si, error_string
    call print
    jmp $

not_found_error:
    mov si, not_found_msg
    call print
    jmp $

print:
    mov ah, 0x0e
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

entry_string:   db "This is NTH by KARTAVYA SHUKLA.", 0x0d, 0x0a, 0
error_string:   db "Disk Read Error!", 0x0d, 0x0a, 0
kernel_filename db "KERNEL  BIN"
not_found_msg:  db "KERNEL.BIN missing!", 0x0d, 0x0a, 0

absolute_track  db 0
absolute_sector db 0
absolute_head   db 0
kernel_cluster  dw 0

gdt_start:
gdt_null:
    dq 0x0
gdt_code:
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
gdt_data:
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    mov esi, pm_string
    mov edi, 0xB8000
.print_vga:
    lodsb
    test al, al
    jz .execute_kernel
    mov [edi], al
    inc edi
    mov byte [edi], 0x0F
    inc edi
    jmp .print_vga

.execute_kernel:
    jmp 0x1000

pm_string: db "Successfully entered 32-bit Protected Mode!", 0

times 510 - ($-$$) db 0
dw 0xaa55