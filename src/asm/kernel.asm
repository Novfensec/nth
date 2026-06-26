[bits 32]

start:
    mov edi, 0xB80A0
    mov byte [edi], 'K'
    mov byte [edi+1], 0x0A
    mov byte [edi+2], 'E'
    mov byte [edi+3], 0x0A
    mov byte [edi+4], 'R'
    mov byte [edi+5], 0x0A
    mov byte [edi+6], 'N'
    mov byte [edi+7], 0x0A
    mov byte [edi+8], 'E'
    mov byte [edi+9], 0x0A
    mov byte [edi+10], 'L'
    mov byte [edi+11], 0x0A
    
    cli
    hlt