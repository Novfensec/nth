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
    
    in al, 0x92
    or al, 2
    out 0x92, al

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

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

entry_string: db "This is NTH by KARTAVYA SHUKLA. Booting... Switching to 32-bit PM.", 0x0d, 0x0a, 0

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
    jz .halt
    mov [edi], al
    inc edi
    mov byte [edi], 0x0F
    inc edi
    jmp .print_vga

.halt:
    cli
    hlt

pm_string: db "Successfully entered 32-bit Protected Mode!", 0

times 510 - ($-$$) db 0
dw 0xaa55
