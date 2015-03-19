include_directories (.
                     C60_DLOAD_REL
                     C60_DLOAD_DYN
                     DLOAD_SYM
                     DLOAD
                     DLOAD_API
                     DLWRAPPER
                    )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -DC60_TARGET -DLOADER_DEBUG -O3 -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast")

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

SET(LIBRARY_OUTPUT_PATH ${Coal_BINARY_DIR}/lib)

TARGET_LINK_LIBRARIES(oclload
    /db/sds14/user2/al/desktop-linux-sdk/sdk/driver/lib/drv_api.a
)


install(TARGETS oclload ARCHIVE DESTINATION ${Coal_BINARY_DIR}/lib)
