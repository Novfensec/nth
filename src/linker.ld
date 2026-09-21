ENTRY(kernel_main)

SECTIONS {
    . = 0x100000;

    .text : ALIGN(0x1000) {
        *(.text)
    }
    .rodata : ALIGN(0x1000) {
        *(.rodata)
    }
    .data : ALIGN(0x1000) {
        *(.data)
    }
    .bss : ALIGN(0x1000) {
        *(COMMON)
        *(.bss)
    }
}
