# Create and install the OpenCL RTSC module

if (${BUILD_OS} MATCHES "SYS_BIOS")
    install(CODE "execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ti/opencl)")
    if (${BUILD_TARGET} MATCHES "ARM_AM57")
        install(CODE "execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ti/opencl/platforms/am57x_rtos)")
    endif()
    install(DIRECTORY ti/opencl/ DESTINATION /. ${OCL_DPERMS})
endif()
