add_custom_target(init ALL make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(FILES init_dspc8681.out DESTINATION /usr/share/ti/opencl ${OCL_FPERMS})
install(FILES init_dspc8682.out DESTINATION /usr/share/ti/opencl ${OCL_FPERMS})
