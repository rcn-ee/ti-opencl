# Build monitor_ipu

if (BUILD_OUTPUT STREQUAL "all")
    install(CODE "execute_process(COMMAND make -f Makefile BUILD_AM57=1 BUILD_DLA_FIRMWARE=${BUILD_DLA_FIRMWARE} WORKING_DIRECTORY ${OCL_MONITOR_IPU_DIR})")
endif()

# Install monitor_eve firmware
if (BUILD_OUTPUT STREQUAL "all")
    install(FILES bin/release/server_ipu1.xem4 DESTINATION /lib/firmware
            RENAME dra7-ipu1-fw.xem4.opencl-monitor ${OCL_FPERMS})
endif()
