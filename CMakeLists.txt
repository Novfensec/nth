cmake_minimum_required(VERSION 3.15)
project(nth NONE)

find_program(NASM_EXECUTABLE nasm REQUIRED)

find_program(DD_EXECUTABLE dd
    PATHS 
    "C:/Program Files/Git/usr/bin"
    "C:/Program Files/Git/bin"
)

find_program(MTOOLS_MFORMAT mformat)
find_program(MTOOLS_MCOPY mcopy)

set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(IMAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/osimages")
file(MAKE_DIRECTORY ${OUTPUT_DIR} ${IMAGE_DIR})

set(BOOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/asm/boot.asm")
set(KERNEL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/asm/kernel.asm")

set(BOOT_BIN "${OUTPUT_DIR}/nth.bin")
set(KERNEL_BIN "${OUTPUT_DIR}/kernel.bin")
set(OS_IMAGE "${IMAGE_DIR}/nth_os.img")

add_custom_command(
    OUTPUT ${BOOT_BIN}
    COMMAND ${NASM_EXECUTABLE} -f bin ${BOOT_SRC} -o ${BOOT_BIN}
    DEPENDS ${BOOT_SRC}
    COMMENT "Assembling bootloader..."
    VERBATIM
)

add_custom_command(
    OUTPUT ${KERNEL_BIN}
    COMMAND ${NASM_EXECUTABLE} -f bin ${KERNEL_SRC} -o ${KERNEL_BIN}
    DEPENDS ${KERNEL_SRC}
    COMMENT "Assembling kernel..."
    VERBATIM
)

if(DD_EXECUTABLE AND MTOOLS_MFORMAT AND MTOOLS_MCOPY)
    add_custom_command(
        OUTPUT ${OS_IMAGE}
        COMMAND ${DD_EXECUTABLE} if=/dev/zero of=${OS_IMAGE} bs=512 count=2880 status=none
        COMMAND ${MTOOLS_MFORMAT} -i ${OS_IMAGE} -f 1440 ::
        COMMAND ${DD_EXECUTABLE} if=${BOOT_BIN} of=${OS_IMAGE} bs=512 count=1 conv=notrunc status=none
        COMMAND ${MTOOLS_MCOPY} -i ${OS_IMAGE} ${KERNEL_BIN} ::/KERNEL.BIN
        DEPENDS ${BOOT_BIN} ${KERNEL_BIN}
        COMMENT "Generating and populating FAT12 disk image..."
        VERBATIM
    )
else()
    add_custom_command(
        OUTPUT ${OS_IMAGE}
        COMMAND ${CMAKE_COMMAND} -E rm -f ${OS_IMAGE}
        COMMAND fsutil file createnew ${OS_IMAGE} 1474560
        COMMAND ${CMAKE_COMMAND} -E echo "========================================================"
        COMMAND ${CMAKE_COMMAND} -E echo " WARNING: 'dd' or 'mtools' not found in PATH."
        COMMAND ${CMAKE_COMMAND} -E echo " You must manually format the image and copy the files."
        COMMAND ${CMAKE_COMMAND} -E echo "========================================================"
        DEPENDS ${BOOT_BIN} ${KERNEL_BIN}
        COMMENT "Created blank image via fsutil. Manual FAT12 population required."
        VERBATIM
    )
endif()

add_custom_target(nth ALL DEPENDS ${OS_IMAGE})

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${OUTPUT_DIR};${IMAGE_DIR}")