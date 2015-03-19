include_directories (.
                     C60_DLOAD_REL
                     C60_DLOAD_DYN
                     DLOAD_SYM
                     DLOAD
                     DLOAD_API
                     DLWRAPPER
                    )

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

