if (C6678_BUILD OR HAWKING_CROSS_COMPILE)
  install(DIRECTORY DESTINATION ${SHARE_PATH}/opencl    ${OCL_DPERMS})
  install(DIRECTORY DESTINATION ${SHARE_PATH}/opencl/bin ${OCL_DPERMS})
  install(DIRECTORY DESTINATION ${SHARE_PATH}/opencl/bin/x86 ${OCL_DPERMS})
  install(PROGRAMS x86/clocl DESTINATION ${SHARE_PATH}/opencl/bin/x86 ${OCL_BPERMS})  
  add_custom_command(OUTPUT x86/clocl  COMMAND make -j4
                    _PRODUCT_VERSION=${${PROJECT_NAME}_VERSION}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_target(x86_clocl DEPENDS x86/clocl)
  set(CROSS_TARGET cross)
endif()

if (HAWKING_BUILD)
 add_custom_command(OUTPUT arm/clocl COMMAND make -j4 ${CROSS_TARGET} TARGET=arm
                    _PRODUCT_VERSION=${${PROJECT_NAME}_VERSION}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
 add_custom_target(arm_clocl  DEPENDS arm/clocl)
 install(PROGRAMS arm/clocl DESTINATION /usr/bin ${OCL_BPERMS})
endif()
