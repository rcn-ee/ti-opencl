include_directories (.
                     C60_DLOAD_REL
                     C60_DLOAD_DYN
                     DLOAD_SYM
                     DLOAD
                     DLOAD_API
                     DLWRAPPER
                    )
if (${BUILD_OS} MATCHES "SYS_BIOS")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-a15 -mtune=cortex-a15 -mfloat-abi=hard -D_SYS_BIOS -I${XDC_INSTALL_PATH} -I${BIOS_INSTALL_PATH} -Dxdc_target_types__=${BIOS_INSTALL_PATH}/gnu/targets/arm/std.h -Dxdc_target_name__=A15F")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -DC60_TARGET -DLOADER_DEBUG -g -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast")

set(OCL_LOAD_SRC_FILES 
    ocl_load.c
    C60_DLOAD_REL/c60_reloc.c
    C60_DLOAD_DYN/c60_dynamic.c
    DLOAD_SYM/symtab.c
    DLOAD/ArrayList.c
    DLOAD/dload.c
    DLOAD/elf32.c
    DLOAD/dload_endian.c
)

add_library(oclload STATIC ${OCL_LOAD_SRC_FILES})

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

