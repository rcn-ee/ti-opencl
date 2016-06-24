cmake_minimum_required(VERSION 2.6)

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeListsWin.txt)
else()

if (C6678_BUILD OR CROSS_COMPILE)
  set (CROSS_TARGET arm)
  install(DIRECTORY DESTINATION ${SHARE_PATH}/opencl    ${OCL_DPERMS})
  install(DIRECTORY DESTINATION ${SHARE_PATH}/opencl/bin ${OCL_DPERMS})
  install(DIRECTORY DESTINATION ${SHARE_PATH}/opencl/bin/x86 ${OCL_DPERMS})
  install(PROGRAMS x86/clocl DESTINATION ${SHARE_PATH}/opencl/bin/x86 ${OCL_BPERMS})  

  
  add_custom_command(OUTPUT x86/clocl  COMMAND make -j4 x86
                    _PRODUCT_VERSION=${${PROJECT_NAME}_PKG_VERSION}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_custom_target(x86_clocl DEPENDS x86/clocl)
endif()

if (K2X_BUILD OR K2G_BUILD OR AM57_BUILD)
 if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
     add_custom_command(OUTPUT arm/clocl COMMAND make -j4 ${CROSS_TARGET}
                        _PRODUCT_VERSION=${${PROJECT_NAME}_PKG_VERSION}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

     add_custom_target(arm_clocl  DEPENDS arm/clocl)
     install(PROGRAMS arm/clocl DESTINATION /usr/bin ${OCL_BPERMS})
 endif()
endif()

endif ()
