cmake_minimum_required(VERSION 3.15)
project(nth NONE)

find_program(NASM_EXECUTABLE nasm REQUIRED)

set(BOOTLOADER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/asm/boot.asm")
set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
file(MAKE_DIRECTORY ${OUTPUT_DIR})
set(BOOTLOADER_BIN "${OUTPUT_DIR}/nth.bin")

add_custom_command(
    OUTPUT ${BOOTLOADER_BIN}
    COMMAND ${NASM_EXECUTABLE} -f bin ${BOOTLOADER_SRC} -o ${BOOTLOADER_BIN}
    DEPENDS ${BOOTLOADER_SRC}
    COMMENT "Assembling nth bootloader..."
    VERBATIM
)

add_custom_target(nth ALL DEPENDS ${BOOTLOADER_BIN})

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${OUTPUT_DIR}")
