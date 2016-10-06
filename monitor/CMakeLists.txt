# Build monitor

if(BUILD_OUTPUT STREQUAL "all" )
    install(CODE "execute_process(COMMAND make -j4 BUILD_TARGET=${BUILD_TARGET} WORKING_DIRECTORY ${OCL_MONITOR_DIR})")
endif()

# Install monitor, linker command file, and symbols

if(BUILD_OUTPUT STREQUAL "all" OR BUILD_OUTPUT STREQUAL "lib")
    if (${BUILD_TARGET} MATCHES "DSPC868x")
        install(FILES monitor_dspc868x/dsp.out monitor_dspc868x/dsp.syms monitor_dspc868x/dsp_syms.obj DESTINATION ${SHARE_PATH}/opencl ${OCL_FPERMS})
    elseif (${BUILD_TARGET} MATCHES "ARM_K2H")
        install(FILES monitor_evmk2h/dsp0.out monitor_evmk2h/dsp1.out monitor_evmk2h/dsp2.out monitor_evmk2h/dsp3.out monitor_evmk2h/dsp4.out monitor_evmk2h/dsp5.out monitor_evmk2h/dsp6.out monitor_evmk2h/dsp7.out DESTINATION ${SHARE_PATH}/opencl ${OCL_FPERMS})
        install(FILES monitor_evmk2h/dsp0.out DESTINATION ${SHARE_PATH}/opencl RENAME dsp.out ${OCL_FPERMS})
        install(FILES monitor_evmk2h/dsp0.syms.obj DESTINATION ${SHARE_PATH}/opencl RENAME dsp_syms.obj ${OCL_FPERMS})
        install(FILES monitor_evmk2h/dsp0.syms DESTINATION ${SHARE_PATH}/opencl RENAME dsp.syms ${OCL_FPERMS})
    elseif (${BUILD_TARGET} MATCHES "ARM_K2L")
        install(FILES monitor_evmk2l/dsp.out monitor_evmk2l/dsp.syms monitor_evmk2l/dsp_syms.obj DESTINATION ${SHARE_PATH}/opencl ${OCL_FPERMS})
    elseif (${BUILD_TARGET} MATCHES "ARM_K2E")
        install(FILES monitor_evmk2e/dsp.out monitor_evmk2e/dsp.syms monitor_evmk2e/dsp_syms.obj DESTINATION ${SHARE_PATH}/opencl ${OCL_FPERMS})
    else()
        message(FATAL_ERROR "invalid BUILD_TARGET ${BUILD_TARGET}")
    endif()
endif()
