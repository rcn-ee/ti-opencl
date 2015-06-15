# Clean the examples directory so that the install package is clean.
# Assuming that opencl_examples directory is present otherwise will get a
# cmake error. 
# NOTE: SHOULD REMOVE ANY INSTALL PACKAGE PRIOR TO DOING A PRODUCTION INSTALL
install(CODE "execute_process(COMMAND make clean WORKING_DIRECTORY ${OCL_EXAMPLES_DIR})")

set (EXAMPLES_PATH    ${SHARE_PATH}/examples)
set (OCL_EXAMPLES_PATH     ${EXAMPLES_PATH}/opencl)
set (OCL_OMP_EXAMPLES_PATH ${EXAMPLES_PATH}/opencl+openmp)

# Install the top-level directories and Makefiles
install(DIRECTORY DESTINATION ${EXAMPLESE_PATH}        ${OCL_DPERMS})
install(DIRECTORY DESTINATION ${OCL_EXAMPLES_PATH}     ${OCL_DPERMS})
install(DIRECTORY DESTINATION ${OCL_OMP_EXAMPLES_PATH} ${OCL_DPERMS})
install(FILES make.inc Makefile DESTINATION ${OCL_EXAMPLES_PATH} ${OCL_FPERMS})
install(FILES make.inc Makefile DESTINATION ${OCL_OMP_EXAMPLES_PATH} ${OCL_FPERMS})

# Explicitly list the examples to be installed. Note that this approach requires
# an update to this file as new examples are added. 
# Initially set this list to the examples that will be installed regardless
# of the target platform.  Use the target build specific blocks below to add to
# this list
SET(OCL_EXAMPLES_INSTALL_LIST edmamgr matmpy simple dsplib_fft mandelbrot mandelbrot_native offline offline_embed platforms ccode null ooo vecadd dspheap float_compute)
SET(OCL_OMP_EXAMPLES_INSTALL_LIST vecadd_openmp vecadd_openmp_t)

# Install hawking specific examples
IF (HAWKING_BUILD)
  # Add any hawking-specific examples
  LIST (APPEND OCL_EXAMPLES_INSTALL_LIST ooo_map vecadd_mpax sgemm)
  LIST (APPEND OCL_OMP_EXAMPLES_INSTALL_LIST vecadd_mpax_openmp dgemm)
ENDIF(HAWKING_BUILD)

# Install C6678 (dspc) specific examples
IF (C6678_BUILD)
  # Add any C6678-specific examples, none currently
  # LIST (APPEND OCL_EXAMPLE_INSTALL_LIST xxx xxx)
ENDIF(C6678_BUILD)

# Loop through the directory list and install
FOREACH(subdir ${OCL_EXAMPLES_INSTALL_LIST})
   install(DIRECTORY ${subdir} DESTINATION ${OCL_EXAMPLES_PATH} ${OCL_DPERMS})
ENDFOREACH()

# Loop through the directory list and install
FOREACH(subdir ${OCL_OMP_EXAMPLES_INSTALL_LIST})
   install(DIRECTORY ${subdir} DESTINATION ${OCL_OMP_EXAMPLES_PATH} ${OCL_DPERMS})
ENDFOREACH()
