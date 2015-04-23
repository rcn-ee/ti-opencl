# Make and install appropriate monitor executable

# Currently the monitor must be built in an X86 cross-compile environment
# If doing an opencl build/install on an ARM platform the monitor files
# must be copied over.
if(CROSS_COMPILE)
   install(CODE "execute_process(COMMAND make WORKING_DIRECTORY ${OCL_MONITOR_DIR})")
endif()

install(FILES dsp0.out  DESTINATION /usr/share/ti/opencl RENAME dsp.out  ${OCL_FPERMS})
install(FILES dsp0.syms DESTINATION /usr/share/ti/opencl RENAME dsp.syms ${OCL_FPERMS})
install(FILES dsp0.out DESTINATION /lib/firmware RENAME dra7-dsp1-fw.xe66 ${OCL_FPERMS})
install(FILES dsp1.out DESTINATION /lib/firmware RENAME dra7-dsp2-fw.xe66 ${OCL_FPERMS})
