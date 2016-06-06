if (C6678_BUILD)
  SET (TARGET_INCLUDES
    ${PROJECT_SOURCE_DIR}/init
    ${SDK}/sdk
    ${SDK}/sdk/config
    ${SDK}/sdk/pciedrv
    ${SDK}/sdk/cmem
    ${SDK}/sdk/bufmgr
    ${SDK}/sdk/mailBox
    ${SDK}/sdk/dnldmgr )
# Cross-compiling needs additional paths to find target OS headers 
# and non-system headers found on the host (BOOST,GL)
elseif (CROSS_COMPILE)
  SET (TARGET_INCLUDES 
    ${CMAKE_FIND_ROOT_PATH} 
    ${HOST_USR_INCLUDE_PATH}  )
endif()


include_directories (
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${LLVM_INCLUDE_DIR}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src/core/dsp/tal/ocl_load/DLOAD_API
    ${PROJECT_SOURCE_DIR}/src/core/dsp/tal/ocl_load/DLOAD
    ${PROJECT_SOURCE_DIR}/src/core/dsp/tal
    ${PROJECT_SOURCE_DIR}/src/core/dsp
    ${TARGET_INCLUDES}
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-literal-suffix")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_PRODUCT_VERSION=${${PROJECT_NAME}_PKG_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (${BUILD_OS} MATCHES "SYS_BIOS")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${XDC_INSTALL_PATH} -I${BIOS_INSTALL_PATH} -I${LOKI_INCLUDE_PATH}")
endif()


# bfd.h has a check to ensure that config.h is included
# We don't require config.h (autotools) so we bypass this check by defining
# PACKAGE, and PACKAGE_VERSION 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPACKAGE=${PROJECT_NAME} -DPACKAGE_VERSION=${${PROJECT_NAME}_PKG_VERSION}")

# Toggle below if wanting to build with debug
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-inline -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if (C6678_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDSPC868X")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DDSPC868X")
endif()

if (K2X_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEVICE_K2X")
endif()

if (K2G_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEVICE_K2G")
endif()

if (AM57_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEVICE_AM57")
endif()

if (ENABLE_ULM)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DULM_ENABLED")
endif()

configure_file(core/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/core/config.h)

set(COAL_SRC_FILES
    api/api_command.cpp
    api/api_device.cpp
    api/api_event.cpp
    api/api_kernel.cpp
    api/api_platform.cpp
    api/api_program.cpp
    api/api_context.cpp
    api/api_enqueue.cpp
    api/api_flush.cpp
    api/api_memory.cpp
    api/api_profiling.cpp
    api/api_sampler.cpp

    core/context.cpp
    core/commandqueue.cpp
    core/memobject.cpp
    core/events.cpp
    core/program.cpp
    core/kernel.cpp
    core/sampler.cpp
    core/object.cpp
    core/platform.cpp
    core/icd.cpp
    core/util.cpp
    core/error_report.cpp

    core/dsp/program.cpp
    core/dsp/driver.cpp
    core/dsp/buffer.cpp
    core/dsp/device.cpp
    core/dsp/kernel.cpp
    core/dsp/worker.cpp
    core/dsp/mmap_resource.cpp
    core/dsp/device_info.cpp

    core/dsp/tal/dload.cpp
    core/dsp/tal/memory_provider_factory.cpp
    core/dsp/tal/shared_memory_provider.cpp
    core/dsp/tal/mbox_factory.cpp
)

if (${BUILD_OS} MATCHES "SYS_BIOS")
set(COAL_SRC_FILES ${COAL_SRC_FILES}
    core/dsp/tal/rtos_shmem_provider_factory.cpp
)
else()
set(COAL_SRC_FILES ${COAL_SRC_FILES}
    api/api_gl.cpp

    core/compiler.cpp

    core/cpu/buffer.cpp
    core/cpu/device.cpp
    core/cpu/kernel.cpp
    core/cpu/program.cpp
    core/cpu/worker.cpp
    core/cpu/builtins.cpp
    core/cpu/sampler.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.h.embed.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.c.bc.embed.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_impl.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_def.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_impl.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_def.h

    core/dsp/genfile_cache.cpp

    core/dsp/tal/shmem_provider_factory.cpp
    core/dsp/tal/symbol_address_elf.cpp
)
endif()

if (K2X_BUILD)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/rm_qmss_res.c)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/mbox_impl_mpm.cpp)
endif()

if (K2X_BUILD OR K2G_BUILD)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/devmem_map_policy_mpm.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/device_manager_mpm.cpp)
endif()

if (K2G_BUILD OR AM57_BUILD)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/mbox_impl_msgq.cpp)
endif()

if (AM57_BUILD)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/device_manager_remoteproc.cpp)
endif()

if (AM57_BUILD AND (NOT (${BUILD_OS} MATCHES "SYS_BIOS")))
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/devmem_map_policy_mmap.cpp)
endif()

if (C6678_BUILD)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_init_policy_pcie.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_rw_policy_pcie.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/memory_provider_pcie.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_pcie.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/pcie_cmem.cpp)
else()
  if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_init_policy_cmem.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_rw_policy_cmem.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/memory_provider_cmem.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_cmem.cpp)
  else()
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_init_policy_rtos.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_rw_policy_rtos.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/memory_provider_rtos.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_rtos.cpp)
  endif()
endif()

add_subdirectory(runtime)

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
add_library(OpenCL SHARED ${COAL_SRC_FILES})
else()
add_library(OpenCL STATIC ${COAL_SRC_FILES})
endif()

if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.h.embed.h
                            PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.c.bc.embed.h
                            PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_impl.h
                            PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_def.h
                            PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_impl.h
                            PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_def.h
                            PROPERTIES GENERATED 1)

add_dependencies(OpenCL generate_stdlib_c)
add_dependencies(OpenCL generate_builtins)
endif()

add_dependencies(OpenCL oclload)
if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
add_dependencies(OpenCL generate_dsp_builtins)
endif()

add_subdirectory(core/dsp/tal/ocl_load)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET_TARGET_PROPERTIES(OpenCL PROPERTIES
  VERSION ${${PROJECT_NAME}_VERSION}
  SOVERSION ${${PROJECT_NAME}_SOVERSION}
)

if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
set_source_files_properties(${COAL_SRC_FILES}
        PROPERTIES COMPILE_FLAGS ${LLVM_COMPILE_FLAGS})

set_target_properties(OpenCL PROPERTIES
  LINK_FLAGS "${LLVM_LDFLAGS}"
  LINK_INTERFACE_LIBRARIES "")
endif()

# Dirty platform.cpp to cause re-compile with every build. It contains
# a compile-time timestamp used as the library build timestamp.
add_custom_command (TARGET OpenCL PRE_BUILD 
                 COMMAND touch -c ${CMAKE_CURRENT_SOURCE_DIR}/core/platform.cpp)

if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
set (LIBS
        #${CLANG_LIBS}
        #${LLVM_LIBS_CORE}
        LLVMBitReader
        LLVMBitWriter
        LLVMCore
        LLVMSupport
        #${LLVM_LIBS_JIT}
	${PROJECT_BINARY_DIR}/lib/liboclload.a
        pthread
        rt
        dl
)

# Add non-target dependent libraries
find_library(FFI_LIB ffi)
find_library(SQLITE3_LIB sqlite3)
find_library(ELF_LIB elf)

LIST (APPEND LIBS ${FFI_LIB} ${SQLITE3_LIB} ${ELF_LIB})

# Add target dependent libraries
if(K2X_BUILD)
   find_library(CMEM_LIB          ticmem)
   find_library(MPMMAIL_LIB       mpmmailbox)
   find_library(MPMTRANSPORT_LIB  mpmtransport)
   find_library(MPMCLIENT_LIB     mpmclient)
   find_library(RM_LIB            rm)

   LIST (APPEND LIBS ${CMEM_LIB} ${MPMMAIL_LIB} ${MPMTRANSPORT_LIB} 
                     ${MPMCLIENT_LIB} ${RM_LIB} )
elseif(C6678_BUILD)
  LIST (APPEND LIBS
	${SDK}/sdk/pciedrv/lib/pciedrv.a
	${SDK}/sdk/dnldmgr/lib/dnldmgr.a
	${SDK}/sdk/cmem/lib/cmem_drv.a
	${SDK}/sdk/bufmgr/lib/bufmgr.a
	${SDK}/sdk/mailBox/host/lib/mailBox.a
        pciaccess
	)
elseif (K2G_BUILD)
  find_library(CMEM_LIB          ticmem)
  find_library(IPC_LIB           tiipc)
  find_library(IPC_UTIL_LIB      tiipcutils)
  find_library(IPC_TRANS_RPMSG   titransportrpmsg)
  find_library(MPMTRANSPORT_LIB  mpmtransport)
  find_library(MPMCLIENT_LIB     mpmclient)
  LIST (APPEND LIBS ${CMEM_LIB} ${IPC_LIB} ${IPC_UTIL_LIB} ${IPC_TRANS_RPMSG}
                    ${MPMTRANSPORT_LIB} ${MPMCLIENT_LIB})
elseif (AM57_BUILD)
  find_library(CMEM_LIB          ticmem)
  find_library(IPC_LIB           tiipc)
  find_library(IPC_UTIL_LIB      tiipcutils)
  find_library(IPC_TRANS_RPMSG   titransportrpmsg)
  LIST (APPEND LIBS ${CMEM_LIB} ${IPC_LIB} ${IPC_UTIL_LIB} ${IPC_TRANS_RPMSG})
endif()

if(ENABLE_ULM)
  find_library(ULM_LIB  NAMES    libtiulm.a)
  LIST (APPEND LIBS ${ULM_LIB} )
endif()

TARGET_LINK_LIBRARIES(OpenCL ${LIBS})
endif()  # if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))

if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
  install(TARGETS OpenCL LIBRARY DESTINATION /usr/lib ${OCL_FPERMS})
else()
  file(WRITE ${PROJECT_BINARY_DIR}/lib/OpenCLAddLib.mri
    "OPEN ../lib/libOpenCL.a
    ADDLIB ${ARM_LLVM_DIR}/lib/libLLVMBitReader.a
    ADDLIB ${ARM_LLVM_DIR}/lib/libLLVMCore.a
    ADDLIB ${ARM_LLVM_DIR}/lib/libLLVMSupport.a
    ADDLIB ${PROJECT_BINARY_DIR}/lib/liboclload.a
    SAVE
    END")
  add_custom_command (TARGET OpenCL POST_BUILD
    COMMAND ${CMAKE_AR} -M < ${PROJECT_BINARY_DIR}/lib/OpenCLAddLib.mri)
  install(TARGETS OpenCL ARCHIVE DESTINATION /usr/lib ${OCL_FPERMS})
endif()

