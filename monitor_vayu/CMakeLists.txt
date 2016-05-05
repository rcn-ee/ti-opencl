# Make and install appropriate monitor executable

# Currently the monitor must be built in an X86 cross-compile environment
# If doing an opencl build/install on an ARM platform the monitor files
# must be copied over.
if(CROSS_COMPILE)
    if (${BUILD_TARGET} MATCHES "ARM_AM57")
       if (${BUILD_OS} MATCHES "SYS_BIOS")
           install(CODE "execute_process(COMMAND make -f Makefile.am57x_rtos -j4 BUILD_TARGET=${BUILD_TARGET} WORKING_DIRECTORY ${OCL_MONITOR_DIR})")
       else()
           install(CODE "execute_process(COMMAND make -f Makefile -j4 BUILD_TARGET=${BUILD_TARGET} WORKING_DIRECTORY ${OCL_MONITOR_DIR})")
       endif()
    elseif (${BUILD_TARGET} MATCHES "ARM_K2G")
       install(CODE "execute_process(COMMAND make -f Makefile.evmk2g -j4 BUILD_TARGET=${BUILD_TARGET} WORKING_DIRECTORY ${OCL_MONITOR_DIR})")
    else()
        message(FATAL_ERROR "invalid BUILD_TARGET ${BUILD_TARGET}")
    endif()
endif()

if (${BUILD_TARGET} MATCHES "ARM_AM57")
    install(FILES dsp0.out  DESTINATION /usr/share/ti/opencl RENAME dsp.out  ${OCL_FPERMS})
    install(FILES dsp0.syms DESTINATION /usr/share/ti/opencl RENAME dsp.syms ${OCL_FPERMS})
    install(FILES dsp0.syms.obj DESTINATION /usr/share/ti/opencl RENAME dsp_syms.obj ${OCL_FPERMS})
    if (NOT ${BUILD_OS} MATCHES "SYS_BIOS")
    install(FILES dsp0.out DESTINATION /lib/firmware RENAME dra7-dsp1-fw.xe66 ${OCL_FPERMS})
    install(FILES dsp1.out DESTINATION /lib/firmware RENAME dra7-dsp2-fw.xe66 ${OCL_FPERMS})
    endif()
elseif (${BUILD_TARGET} MATCHES "ARM_K2G")
    install(FILES monitor_evmk2g/dsp.out  DESTINATION /usr/share/ti/opencl ${OCL_FPERMS})
    install(FILES monitor_evmk2g/dsp.syms DESTINATION /usr/share/ti/opencl ${OCL_FPERMS})
    install(FILES monitor_evmk2g/dsp_syms.obj DESTINATION /usr/share/ti/opencl ${OCL_FPERMS})
else()
    message(FATAL_ERROR "invalid BUILD_TARGET ${BUILD_TARGET}")
endif()
