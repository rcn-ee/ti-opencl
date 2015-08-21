cmake_minimum_required(VERSION 2.6)
#Project needs to be set early or CMAKE_SYSTEM_PROCESSOR doesn't get 
#reliably set. Commit to C6678 and then change below if we build for HAWKING
project(ti-opencl)

if (${BUILD_TARGET} MATCHES "DSPC868x")
    set(C6678_BUILD on)
    set(SDK "/opt/ti/desktop-linux-sdk_01_00_03_00")
    MESSAGE(STATUS "Build Target Is C6678")
elseif (${BUILD_TARGET} MATCHES "ARM_K2H")
    set(HAWKING_BUILD on)
    MESSAGE(STATUS "Build Target Is  HAWKING")
elseif (${BUILD_TARGET} MATCHES "ARM_AM57")
    set(AM57_BUILD on)
    MESSAGE(STATUS "Build Target Is  AM57")
else()
    MESSAGE(FATAL_ERROR "No build target selected. Set cmake variable BUILD_TARGET")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# project version
# The linux shared library versioning convention uses x.y.z format,
# with no leading zeros. For debian packaging versioning we use xx.yy.zz.ww
# formatting with leading zero's.  Opencl/clocl platform and version queries
# will report in this format.  The 'ww' represents the PATCH_VERSION of a
# particular release.  This number will not be reflected in the library
# .so name, only in the packaging.
# NOTE!!!!!! The packaging version numbers need to match what is specified in 
# debian/changelog!!!
SET(OCL_SPEC_MAJOR_VERSION 1)
SET(OCL_SPEC_MINOR_VERSION 1)
SET(RELEASE_VERSION 6)
# use 2-digits to specify patch version as there is no need to convert
# to library versioning
SET(PATCH_VERSION   00)

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

OPTION(BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON)

#ADD_DEFINITIONS("-DLOCK")

#SET(CMAKE_VERBOSE_MAKEFILE TRUE)

# Find_Package() for LLVM/CLANG defined in CMAKE_MODULE_PATH
# Do not use system installed versions
Find_Package(LLVM REQUIRED)
Find_Package(Clang REQUIRED)

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
    if (HAWKING_BUILD OR C6678_BUILD)
     set(OCL_MONITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../monitor)
    else()
     set(OCL_MONITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../monitor_vayu)
    endif()
endif()

if (NOT OCL_BUILTINS_DIR)
set(OCL_BUILTINS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../builtins)
endif()

# Create the top level share install directory with the desired permissions
set (SHARE_PATH /usr/share/ti)
install(DIRECTORY DESTINATION ${SHARE_PATH} ${OCL_DPERMS})

# For now, pick default ULM configurations for each target
if(NOT DEFINED ENABLE_ULM)
  if (HAWKING_BUILD)
    set(ENABLE_ULM on)
  elseif(C6678_BUILD)
    set(ENABLE_ULM off)
  elseif(AM57_BUILD)
    set(ENABLE_ULM on)
  endif()
endif()

if (NOT BUILD_OUTPUT)
  set(BUILD_OUTPUT all)
endif()

if (${BUILD_OUTPUT} STREQUAL "all")
  add_subdirectory(${OCL_MONITOR_DIR} ${CMAKE_CURRENT_BINARY_DIR}/monitor)
endif()

if (${BUILD_OUTPUT} STREQUAL "all" OR ${BUILD_OUTPUT} STREQUAL "lib")
  add_subdirectory(src)
  add_subdirectory(util)
  add_subdirectory(${OCL_BUILTINS_DIR} ${CMAKE_CURRENT_BINARY_DIR}/builtins)

  install(DIRECTORY DESTINATION /usr/share/doc/ti-opencl         ${OCL_DPERMS})
  install(DIRECTORY DESTINATION /usr/include/CL                  ${OCL_DPERMS})
  install(FILES opencl-manifest.html  DESTINATION /usr/share/doc/ti-opencl 
                                                                 ${OCL_FPERMS})
  install(FILES     util/ocl_util.h  DESTINATION /usr/include    ${OCL_FPERMS})
  install(DIRECTORY include/CL       DESTINATION /usr/include    ${OCL_DPERMS})

  if (C6678_BUILD)
    install(DIRECTORY DESTINATION init                           ${OCL_DPERMS})
    install(FILES init/init_dspc8681.out DESTINATION lib         ${OCL_FPERMS})
    install(FILES init/init_dspc8682.out DESTINATION lib         ${OCL_FPERMS})
    install(PROGRAMS bin/oclenv DESTINATION bin                  ${OCL_BPERMS})
  endif()
endif()

if (BUILD_OUTPUT STREQUAL "all" OR BUILD_OUTPUT STREQUAL "examples")
  add_subdirectory(${OCL_EXAMPLES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/examples)
endif()

if (BUILD_OUTPUT STREQUAL "all" OR BUILD_OUTPUT STREQUAL "clocl")
  add_subdirectory(clocl)

  if (HAWKING_BUILD OR AM57_BUILD)
    add_custom_target(arm_clocl_build ALL DEPENDS arm_clocl)
  endif()

  if (CROSS_COMPILE OR C6678_BUILD)
    add_custom_target(x86_clocl_build ALL DEPENDS x86_clocl)
  endif()
endif()


IF (BUILD_TESTS)
    ENABLE_TESTING()
    Find_Package(Check REQUIRED)
    add_subdirectory(tests)
ENDIF (BUILD_TESTS)

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
