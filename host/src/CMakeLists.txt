include_directories (${Coal_SOURCE_DIR}/include
                    ${Coal_SOURCE_DIR}/src
                    ${LLVM_INCLUDE_DIR}
                    ${CLANG_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${Coal_SOURCE_DIR}/ocl_load/DLOAD_API
                    ${Coal_SOURCE_DIR}/ocl_load/DLOAD
		    $ENV{SDK}/sdk
		    $ENV{SDK}/sdk/config
		    $ENV{SDK}/sdk/pciedrv
		    $ENV{SDK}/sdk/mailBox
		    $ENV{SDK}/sdk/cmem
		    $ENV{SDK}/sdk/bufmgr
		    $ENV{SDK}/sdk/dnldmgr
                    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive")

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
    api/api_gl.cpp

    core/context.cpp
    core/commandqueue.cpp
    core/memobject.cpp
    core/events.cpp
    core/program.cpp
    core/compiler.cpp
    core/kernel.cpp
    core/sampler.cpp
    core/object.cpp
    core/platform.cpp

    core/cpu/buffer.cpp
    core/cpu/device.cpp
    core/cpu/kernel.cpp
    core/cpu/program.cpp
    core/cpu/worker.cpp
    core/cpu/builtins.cpp
    core/cpu/sampler.cpp

    core/dsp/driver.cpp
    core/dsp/buffer.cpp
    core/dsp/device.cpp
    core/dsp/kernel.cpp
    core/dsp/program.cpp
    core/dsp/worker.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.h.embed.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib.c.bc.embed.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_impl.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/builtins_def.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_impl.h
    ${CMAKE_CURRENT_BINARY_DIR}/runtime/stdlib_def.h
)

add_subdirectory(runtime)

add_library(OpenCL SHARED ${COAL_SRC_FILES})

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

add_dependencies(OpenCL generate_stdlib_h)
add_dependencies(OpenCL generate_stdlib_c)
add_dependencies(OpenCL generate_builtins)
add_dependencies(OpenCL oclload)

SET(LIBRARY_OUTPUT_PATH ${Coal_BINARY_DIR}/lib)

SET_TARGET_PROPERTIES(OpenCL PROPERTIES
  VERSION ${${PROJECT_NAME}_VERSION}
  SOVERSION ${${PROJECT_NAME}_SOVERSION}
)

set_source_files_properties(${COAL_SRC_FILES}
        PROPERTIES COMPILE_FLAGS ${LLVM_COMPILE_FLAGS})

set_target_properties(OpenCL PROPERTIES
  LINK_FLAGS ${LLVM_LDFLAGS}
  LINK_INTERFACE_LIBRARIES "")

TARGET_LINK_LIBRARIES(OpenCL
        ${CLANG_LIBS}
        ${LLVM_LIBS_CORE}
        ${LLVM_LIBS_JIT}
	$ENV{SDK}/sdk/pciedrv/lib/pciedrv.a
	$ENV{SDK}/sdk/mailBox/host/lib/mailBox.a
	$ENV{SDK}/sdk/dnldmgr/lib/dnldmgr.a
	$ENV{SDK}/sdk/cmem/lib/cmem_drv.a
	$ENV{SDK}/sdk/bufmgr/lib/bufmgr.a
	${Coal_BINARY_DIR}/lib/liboclload.a
        pthread
        rt
        dl
        ffi
        pciaccess
)

install(TARGETS OpenCL LIBRARY DESTINATION $(Coal_BINARY_DIR)/lib)
