execute_process(COMMAND git rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_SHA
                OUTPUT_STRIP_TRAILING_WHITESPACE)

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
    ${CMAKE_FIND_ROOT_PATH}
    )

if ((${BUILD_OS} MATCHES "SYS_BIOS"))
    include_directories (${BIOS_USR_INCLUDE_PATH})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-literal-suffix")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
# https://gcc.gnu.org/ml/gcc/2017-05/msg00073.html
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_PRODUCT_VERSION=${${PROJECT_NAME}_PKG_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_SYSTEM_NO_DEPRECATED=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_SYSTEM_NO_LIB=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_BUILD_ID=${GIT_SHA}")

if (${SHMEM_MANAGER} MATCHES "ION")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_ION")
endif()

# bfd.h has a check to ensure that config.h is included
# We don't require config.h (autotools) so we bypass this check by defining
# PACKAGE, and PACKAGE_VERSION 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPACKAGE=${PROJECT_NAME} -DPACKAGE_VERSION=${${PROJECT_NAME}_PKG_VERSION}")

# Toggle below if wanting to build with debug
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fno-inline -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if (K2X_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEVICE_K2X")
endif()

if (K2H_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEVICE_K2H")
elseif (K2L_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEVICE_K2L")
elseif (K2E_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEVICE_K2E")
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
    core/oclenv.cpp
    core/kernelentry.cpp
    core/builtinprogram.cpp
    core/builtinkernel.cpp

    core/dsp/program.cpp
    core/dsp/driver.cpp
    core/dsp/buffer.cpp
    core/dsp/device.cpp
    core/dsp/subdevice.cpp
    core/dsp/rootdevice.cpp
    core/dsp/kernel.cpp
    core/dsp/worker.cpp
    core/dsp/mmap_resource.cpp
    core/dsp/device_info.cpp

    core/dsp/tal/dload.cpp
    core/dsp/tal/mbox_factory.cpp
)

if (${SHMEM_MANAGER} MATCHES "CMEM")
set(COAL_SRC_FILES ${COAL_SRC_FILES}
    core/dsp/tal/memory_provider_factory.cpp
    core/dsp/tal/shared_memory_provider.cpp)
endif()


if (${BUILD_OS} MATCHES "SYS_BIOS")
    list (APPEND COAL_SRC_FILES core/dsp/tal/rtos_shmem_provider_factory.cpp)
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
    if (AM57_BUILD)
        set(COAL_SRC_FILES ${COAL_SRC_FILES}
            core/eve/device.cpp
            core/eve/worker.cpp
            core/eve/program.cpp
            core/eve/kernel.cpp
           )
    endif()
endif()

set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/mbox_impl_msgq.cpp)


if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
    if (${SHMEM_MANAGER} MATCHES "CMEM")
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_init_policy_cmem.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_rw_policy_cmem.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/memory_provider_cmem.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_cmem.cpp)
elseif (${SHMEM_MANAGER} MATCHES "ION")
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/ion_allocator.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/ion_memory_provider.cpp)
    endif()
else()
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_init_policy_rtos.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_rw_policy_rtos.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/memory_provider_rtos.cpp)
    set(COAL_SRC_FILES ${COAL_SRC_FILES} core/dsp/tal/shmem_rtos.cpp)
endif()

add_subdirectory(runtime)

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic")
if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
add_library(TIOpenCL SHARED ${COAL_SRC_FILES})
else()
add_library(TIOpenCL STATIC ${COAL_SRC_FILES})
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

add_dependencies(TIOpenCL generate_stdlib_c)
add_dependencies(TIOpenCL generate_builtins)
endif()

add_dependencies(TIOpenCL oclload)
add_subdirectory(core/dsp/tal/ocl_load)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET_TARGET_PROPERTIES(TIOpenCL PROPERTIES
  VERSION ${${PROJECT_NAME}_VERSION}
  SOVERSION ${${PROJECT_NAME}_SOVERSION}
)

if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
set_source_files_properties(${COAL_SRC_FILES}
        PROPERTIES COMPILE_FLAGS ${LLVM_COMPILE_FLAGS})

set_target_properties(TIOpenCL PROPERTIES
  LINK_FLAGS "${LLVM_LDFLAGS}"
  LINK_INTERFACE_LIBRARIES "")
endif()

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
find_library(JSON_LIB json-c)

LIST (APPEND LIBS ${FFI_LIB} ${SQLITE3_LIB} ${ELF_LIB} ${JSON_LIB})

# Add target dependent libraries
if(K2X_BUILD OR K2G_BUILD)
   find_library(MPMTRANSPORT_LIB  mpmtransport)
   find_library(MPMCLIENT_LIB     mpmclient)
   find_library(RM_LIB            rm)

   find_library(IPC_LIB           tiipc)
   find_library(IPC_UTIL_LIB      tiipcutils)
   find_library(IPC_TRANS_RPMSG   titransportrpmsg)

   LIST (APPEND LIBS ${IPC_LIB} ${IPC_UTIL_LIB} ${IPC_TRANS_RPMSG}
                     ${MPMTRANSPORT_LIB} 
                     ${MPMCLIENT_LIB} ${RM_LIB} )
elseif (AM57_BUILD)
  find_library(IPC_LIB           tiipc)
  find_library(IPC_UTIL_LIB      tiipcutils)
  find_library(IPC_TRANS_RPMSG   titransportrpmsg)
  LIST (APPEND LIBS ${IPC_LIB} ${IPC_UTIL_LIB} ${IPC_TRANS_RPMSG})
endif()

if (${SHMEM_MANAGER} MATCHES "CMEM")
    find_library(CMEM_LIB ticmem)
    LIST (APPEND LIBS ${CMEM_LIB} )
endif()

if(ENABLE_ULM)
  find_library(ULM_LIB  NAMES    libtiulm.a)
  LIST (APPEND LIBS ${ULM_LIB} )
endif()

TARGET_LINK_LIBRARIES(TIOpenCL ${LIBS})
endif()  # if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))

if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
  install(TARGETS TIOpenCL LIBRARY DESTINATION /usr/lib ${OCL_FPERMS})
else()
  file(WRITE ${PROJECT_BINARY_DIR}/lib/TIOpenCLAddLib.mri
    "OPEN ../lib/libTIOpenCL.a
    ADDLIB ${ARM_LLVM_DIR}/lib/libLLVMBitReader.a
    ADDLIB ${ARM_LLVM_DIR}/lib/libLLVMCore.a
    ADDLIB ${ARM_LLVM_DIR}/lib/libLLVMSupport.a
    ADDLIB ../lib/liboclload.a
    SAVE
    END")
  add_custom_command (TARGET TIOpenCL POST_BUILD
    COMMAND ${CMAKE_AR} -M < ${PROJECT_BINARY_DIR}/lib/TIOpenCLAddLib.mri)
  install(TARGETS TIOpenCL ARCHIVE DESTINATION /usr/lib ${OCL_FPERMS})
endif()

