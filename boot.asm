[org 0x7c00]

start:
    mov si, entry_string
    call print
    jmp exit

print:
    mov ah, 0x0e
.loop:
    mov al, [si]
    cmp al, 0
    je .done
    inc si
    int 0x10
    jmp .loop
.done:
    ret

exit:
    mov si, exit_string
    call print
    mov ah, 0x00
    int 0x16
    cmp al, 'q'
    je reboot
    cmp al, 'Q'
    je reboot
    jmp exit

reboot:
    jmp 0xffff:0x0000
    

entry_string: db "This is nth by KARTAVYA SHUKLA. Entrypoint initialized...", 0x0d, 0x0a, 0
exit_string: db "Press q (quit) to exit...", 0x0d, 0x0a, 0

times 510 - ($ - $$) db 0
dw 0xaa55
