cmake_minimum_required(VERSION 2.6)
#Project needs to be set early or CMAKE_SYSTEM_PROCESSOR doesn't get 
#reliably set.
project(ti-opencl)

if (${BUILD_TARGET} MATCHES "ARM_K2H")
    set(K2X_BUILD on)
    set(K2H_BUILD on)
    MESSAGE(STATUS "Build Target Is K2x (Hawking)")
elseif (${BUILD_TARGET} MATCHES "ARM_K2L")
    set(K2X_BUILD on)
    set(K2L_BUILD on)
    MESSAGE(STATUS "Build Target Is K2x (Lamarr)")
elseif (${BUILD_TARGET} MATCHES "ARM_K2E")
    set(K2X_BUILD on)
    set(K2E_BUILD on)
    MESSAGE(STATUS "Build Target Is K2x (Edison)")
elseif (${BUILD_TARGET} MATCHES "ARM_K2G")
    set(K2G_BUILD on)
    MESSAGE(STATUS "Build Target Is K2G (Galileo)")
elseif (${BUILD_TARGET} MATCHES "ARM_AM57")
    set(AM57_BUILD on)
    MESSAGE(STATUS "Build Target Is  AM57")
else()
    MESSAGE(FATAL_ERROR "No build target selected. Set cmake variable BUILD_TARGET")
endif()

if (${SHMEM_MANAGER} MATCHES "ION")
    MESSAGE(STATUS "Using ION")
endif()

# Suppress the "Up-to-date" messages from install
set(CMAKE_INSTALL_MESSAGE LAZY)

# Uncomment to generate compile_commands.json file
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# OCL_VERSION defined in Makefile.inc
if (OCL_VERSION)
    string(REPLACE "." ";" OCL_PROJECT_VERSION ${OCL_VERSION})
    list(GET OCL_PROJECT_VERSION 0 OCL_SPEC_MAJOR_VERSION)
    list(GET OCL_PROJECT_VERSION 1 OCL_SPEC_MINOR_VERSION)
    list(GET OCL_PROJECT_VERSION 2 RELEASE_VERSION)
    list(GET OCL_PROJECT_VERSION 3 PATCH_VERSION)
else()
    message(ERROR "cmake variable OCL_VERSION not specified")
endif()

# Create the packaging version numbers which need leading zeros if single digit.
if("${OCL_SPEC_MAJOR_VERSION}" MATCHES "^[0-9]$")
   SET(PKG_MAJOR_VERSION "0${OCL_SPEC_MAJOR_VERSION}")
else()
   SET(PKG_MAJOR_VERSION ${OCL_SPEC_MAJOR_VERSION})
endif()

if("${OCL_SPEC_MINOR_VERSION}" MATCHES "^[0-9]$")
   SET(PKG_MINOR_VERSION "0${OCL_SPEC_MINOR_VERSION}")
else()
   SET(PKG_MINOR_VERSION ${OCL_SPEC_MINOR_VERSION})
endif()

if("${RELEASE_VERSION}" MATCHES "^[0-9]$")
   SET(PKG_RELEASE_VERSION "0${RELEASE_VERSION}")
else()
   SET(PKG_RELEASE_VERSION ${RELEASE_VERSION})
endif()

# Set OpenCL library version number
SET(${PROJECT_NAME}_VERSION ${OCL_SPEC_MAJOR_VERSION}.${OCL_SPEC_MINOR_VERSION}.${RELEASE_VERSION})

# Set SONAME for OpenCL library
SET(${PROJECT_NAME}_SOVERSION ${OCL_SPEC_MAJOR_VERSION})

# Set OpenCL package version
SET(${PROJECT_NAME}_PKG_VERSION ${PKG_MAJOR_VERSION}.${PKG_MINOR_VERSION}.${PKG_RELEASE_VERSION}.${PATCH_VERSION})

MESSAGE(STATUS "Install path is ${CMAKE_INSTALL_PREFIX}")

if (CROSS_COMPILE)
  set(CMAKE_SKIP_RPATH TRUE)
endif()

if (${BUILD_OS} MATCHES "SYS_BIOS")
  OPTION(BUILD_SHARED_LIBS "Set to OFF to build static libraries" OFF)
else()
  OPTION(BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON)
endif()

#ADD_DEFINITIONS("-DLOCK")

#SET(CMAKE_VERBOSE_MAKEFILE TRUE)

# Find_Package() for LLVM/CLANG defined in CMAKE_MODULE_PATH
# Do not use system installed versions
Find_Package(LLVM REQUIRED)
if (${BUILD_OS} MATCHES "SYS_BIOS")
  MESSAGE(STATUS "Build OS IS SYS BIOS")
else()
  Find_Package(Clang REQUIRED)
endif()

# OpenCL requires boost headers. If boost is installed to some directory
# other than /usr/include then define BOOST_INCLUDEDIR as below:
#set (BOOST_INCLUDEDIR path-to-boost-headers)
#include_directories(${BOOST_INCLUDEDIR})

SET (INCLUDE_INSTALL_DIR "include" CACHE PATH
  "The directory the headers are installed in")

# Set up install permissions for file, directories and binaries.
# !!! Note that these variables are used by the child cmake files so these
# variables must be set before the add_subdirectories() commands below
# File permissions (664)
set(OCL_FPERMS PERMISSIONS 
               OWNER_READ OWNER_WRITE 
	       GROUP_READ GROUP_WRITE 
	       WORLD_READ)

# Directory permissions (775). Files within the directories are set as above.
set(OCL_DPERMS FILE_${OCL_FPERMS} 
               DIRECTORY_PERMISSIONS 
	       OWNER_READ OWNER_WRITE OWNER_EXECUTE 
	       GROUP_READ GROUP_WRITE GROUP_EXECUTE 
	       WORLD_READ WORLD_EXECUTE)

# Binary permissions (775)
set(OCL_BPERMS PERMISSIONS 
               OWNER_READ OWNER_WRITE OWNER_EXECUTE 
	       GROUP_READ GROUP_WRITE GROUP_EXECUTE 
	       WORLD_READ WORLD_EXECUTE)


if (NOT OCL_EXAMPLES_DIR)
set(OCL_EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../examples)
endif()

if (NOT OCL_MONITOR_DIR)
     set(OCL_MONITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../monitor)
endif()

if (NOT OCL_MONITOR_IPU_DIR)
     set(OCL_MONITOR_IPU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../monitor_ipu)
endif()

if (NOT OCL_BUILTINS_DIR)
set(OCL_BUILTINS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../builtins)
endif()

# Create the top level share install directory with the desired permissions
set (SHARE_PATH /usr/share/ti)
install(DIRECTORY DESTINATION ${SHARE_PATH} ${OCL_DPERMS})

# For now, pick default ULM configurations for each target
if(NOT DEFINED ENABLE_ULM)
  if (K2X_BUILD)
    set(ENABLE_ULM on)
  elseif(K2G_BUILD)
    set(ENABLE_ULM off)
  elseif(AM57_BUILD)
    if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
      set(ENABLE_ULM on)
    else()
      set(ENABLE_ULM off)
    endif()
  endif()
endif()

if (NOT BUILD_OUTPUT)
  set(BUILD_OUTPUT all)
endif()

if (${BUILD_OS} MATCHES "SYS_BIOS")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../packages ${CMAKE_CURRENT_BINARY_DIR}/packages)
endif(${BUILD_OS} MATCHES "SYS_BIOS")

if (${BUILD_OUTPUT} STREQUAL "all")
  add_subdirectory(${OCL_MONITOR_DIR} ${CMAKE_CURRENT_BINARY_DIR}/monitor)
  if (AM57_BUILD AND (NOT (${BUILD_OS} MATCHES "SYS_BIOS")))
    add_subdirectory(${OCL_MONITOR_IPU_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}/monitor_ipu)
  endif()
endif()

if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
  add_subdirectory(mct-daemon)
endif()

if (${BUILD_OUTPUT} STREQUAL "all" OR ${BUILD_OUTPUT} STREQUAL "lib")
  add_subdirectory(src)
  add_subdirectory(util)
  add_subdirectory(${OCL_BUILTINS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/builtins)

  if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
    install(DIRECTORY DESTINATION /usr/share/doc/ti-opencl ${OCL_DPERMS})
    install(FILES ../opencl-manifest.html  DESTINATION /usr/share/doc/ti-opencl
                                                                 ${OCL_FPERMS})
  else()
    install(DIRECTORY DESTINATION ${DESTDIR}/../../../doc  ${OCL_DPERMS})
    install(FILES ../opencl-manifest.html  DESTINATION ${DESTDIR}/../../../doc
                                                                 ${OCL_FPERMS})
  endif()

  install(DIRECTORY DESTINATION /usr/include/CL                  ${OCL_DPERMS})
  install(FILES     util/ocl_util.h  DESTINATION /usr/include    ${OCL_FPERMS})
  install(DIRECTORY include/CL       DESTINATION /usr/include    ${OCL_DPERMS})

  install(DIRECTORY DESTINATION /usr/share/ti/opencl/profiling   ${OCL_DPERMS})
  install(DIRECTORY ../profiling DESTINATION /usr/share/ti/opencl ${OCL_DPERMS})
  install(PROGRAMS ../profiling/oclaet.sh DESTINATION /usr/share/ti/opencl/profiling ${OCL_BPERMS})

endif()

if (BUILD_OUTPUT STREQUAL "all" OR BUILD_OUTPUT STREQUAL "clocl")
  add_subdirectory(clocl)

  if (K2X_BUILD OR K2G_BUILD OR AM57_BUILD)
    if (NOT (${BUILD_OS} MATCHES "SYS_BIOS"))
        add_custom_target(arm_clocl_build ALL DEPENDS arm_clocl)
    endif()
  endif()

  if (CROSS_COMPILE)
    add_custom_target(x86_clocl_build ALL DEPENDS x86_clocl)
  endif()
endif()

if (BUILD_OUTPUT STREQUAL "all" OR BUILD_OUTPUT STREQUAL "examples")
  add_subdirectory(${OCL_EXAMPLES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/examples)
endif()

IF (BUILD_TESTS)
    ENABLE_TESTING()
    Find_Package(Check REQUIRED)
    add_subdirectory(tests)
endif (BUILD_TESTS)

set (CPACK_GENERATOR "DEB")
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "TI")
set (CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})
set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")

set (CPACK_PACKAGE_VERSION_MAJOR ${PKG_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${PKG_MINOR_VERSION})
set (CPACK_PACKAGE_VERSION_PATCH ${PKG_RELEASE_VERSION})
set (CPACK_DEBIAN_PACKAGE_DEPENDS "mesa-common-dev (>=8.0.4-0), binutils-dev (>=2.22-6), libsqlite3-dev (>=3.7.9-2), libffi6 (>=3.0.11~rc1-5), zlib1g (>=1:1.2.3.4)")

#Set where dpkg will put the install
set (CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set (CPACK_SET_DESTDIR TRUE)
include(CPack)
