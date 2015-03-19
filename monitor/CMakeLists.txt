# Make and install appropriate monitor executable
if (C6678_BUILD)
   install(CODE "execute_process(COMMAND make -j4 WORKING_DIRECTORY ${OCL_MONITOR_DIR})")
elseif(HAWKING_BUILD)
   # Currently the monitor must be built in an X86 cross-compile environment
   # If doing an opencl build/install on an ARM platform the monitor files
   # must be copied over.
  if(HAWKING_CROSS_COMPILE AND BUILD_OUTPUT STREQUAL "all" )
   install(CODE "execute_process(COMMAND make -j4 k2h WORKING_DIRECTORY ${OCL_MONITOR_DIR})")
  endif()

endif()

if(BUILD_OUTPUT STREQUAL "all" OR BUILD_OUTPUT STREQUAL "lib")
  install(FILES dsp.out dsp.syms DESTINATION ${SHARE_PATH}/opencl ${OCL_FPERMS})
endif()