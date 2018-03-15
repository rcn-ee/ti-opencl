# Suppress the "Up-to-date and Install" messages from install
set(CMAKE_INSTALL_MESSAGE NEVER)

# Clean the examples directory so that the install package is clean.
# Assuming that opencl_examples directory is present otherwise will get a
# cmake error. 
# NOTE: SHOULD REMOVE ANY INSTALL PACKAGE PRIOR TO DOING A PRODUCTION INSTALL
install(CODE "execute_process(COMMAND make clean WORKING_DIRECTORY ${OCL_EXAMPLES_DIR})")

if (${BUILD_OS} MATCHES "SYS_BIOS")
    set (EXAMPLES_PATH    /../../../examples)
    set (OCL_EXAMPLES_PATH     ${EXAMPLES_PATH})

    install(DIRECTORY DESTINATION ${OCL_EXAMPLES_PATH}     ${OCL_DPERMS})
else()
    set (EXAMPLES_PATH    ${SHARE_PATH}/examples)
    set (OCL_EXAMPLES_PATH     ${EXAMPLES_PATH}/opencl)
    set (OCL_OMP_EXAMPLES_PATH ${EXAMPLES_PATH}/opencl+openmp)

    # Install the top-level directories and Makefiles
    install(DIRECTORY DESTINATION ${EXAMPLES_PATH}        ${OCL_DPERMS})
    install(DIRECTORY DESTINATION ${OCL_EXAMPLES_PATH}     ${OCL_DPERMS})
    install(DIRECTORY DESTINATION ${OCL_OMP_EXAMPLES_PATH} ${OCL_DPERMS})
endif()

if (${BUILD_OS} MATCHES "SYS_BIOS")
install(FILES rtos_main.c make_rtos.inc Makefile load_am57_rtos.js DESTINATION ${OCL_EXAMPLES_PATH} ${OCL_FPERMS})
else()
install(FILES make.inc Makefile DESTINATION ${OCL_EXAMPLES_PATH} ${OCL_FPERMS})
install(FILES make.inc Makefile DESTINATION ${OCL_OMP_EXAMPLES_PATH} ${OCL_FPERMS})
endif()

# Explicitly list the examples to be installed. Note that this approach requires
# an update to this file as new examples are added. 
# Initially set this list to the examples that will be installed regardless
# of the target platform.  Use the target build specific blocks below to add to
# this list
if (${BUILD_OS} MATCHES "SYS_BIOS")
    SET(OCL_EXAMPLES_INSTALL_LIST abort_exit buffer ccode conv1d
        dsplib_fft edmamgr float_compute
        matmpy null
        offline_embed ooo_callback platforms sgemm simple timeout
        vecadd vecadd_openmp vecadd_openmp_t)
else()
    SET(OCL_EXAMPLES_INSTALL_LIST abort_exit ccode conv1d
        dspheap dsplib_fft edmamgr float_compute
        mandelbrot mandelbrot_native matmpy monte_carlo null
        offline offline_embed ooo platforms sgemm simple timeout
        vecadd)

    # Add persistent examples for AM57/Linux
    if (AM57_BUILD)
        set (OCL_EXAMPLES_INSTALL_LIST ${OCL_EXAMPLES_INSTALL_LIST}
            persistent_clock_concurrent persistent_clock_spanning
            persistent_kernel_timeout persistent_messageq_concurrent
            persistent_task_concurrent persistent_task_spanning
            persistent_common)
        LIST (APPEND OCL_EXAMPLES_INSTALL_LIST custom_device)
        LIST (APPEND OCL_EXAMPLES_INSTALL_LIST tidl)
    endif()
endif()
SET(OCL_OMP_EXAMPLES_INSTALL_LIST vecadd_openmp vecadd_openmp_t dgemm)

# Install hawking specific examples
IF (K2X_BUILD)
  # Add any hawking-specific examples
  LIST (APPEND OCL_EXAMPLES_INSTALL_LIST ooo_map vecadd_mpax)
  LIST (APPEND OCL_OMP_EXAMPLES_INSTALL_LIST vecadd_mpax_openmp)
ENDIF()

# Install C6678 (dspc) specific examples
IF (C6678_BUILD)
  # Add any C6678-specific examples, none currently
  # LIST (APPEND OCL_EXAMPLE_INSTALL_LIST xxx xxx)
ENDIF()

# Loop through the directory list and install
FOREACH(subdir ${OCL_EXAMPLES_INSTALL_LIST})
   install(DIRECTORY ${subdir} DESTINATION ${OCL_EXAMPLES_PATH} ${OCL_DPERMS})
ENDFOREACH()

# Loop through the directory list and install
if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
FOREACH(subdir ${OCL_OMP_EXAMPLES_INSTALL_LIST})
   install(DIRECTORY ${subdir} DESTINATION ${OCL_OMP_EXAMPLES_PATH} ${OCL_DPERMS})
ENDFOREACH()
endif()

# Build installed OpenCL examples (now only for TI-RTOS)
if ((${BUILD_OS} MATCHES "SYS_BIOS") AND (${BUILD_EXAMPLES} MATCHES "1"))
  install(CODE "execute_process(COMMAND make BUILD_OS=SYS_BIOS DEFAULT_PSDK_RTOS=${PSDK_RTOS} TI_OCL_INSTALL=$ENV{DESTDIR} WORKING_DIRECTORY $ENV{DESTDIR}/../../../examples)")
endif()
