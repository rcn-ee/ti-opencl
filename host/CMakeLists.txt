cmake_minimum_required(VERSION 2.6)
project(Coal)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# project version
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 1)
SET(${PROJECT_NAME}_PATCH_LEVEL 0)
# Coal version
SET(${PROJECT_NAME}_VERSION 0.1.0)
# OpenCL version
SET(${PROJECT_NAME}_SOVERSION 1.1.0)


OPTION(BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON)

Find_Package(LLVM REQUIRED)
Find_Package(Clang REQUIRED)

#SET(CMAKE_VERBOSE_MAKEFILE TRUE)
SET(BOOST_ROOT /db/sds13/packages)
Find_Package(Boost 1.38.0 REQUIRED)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

ADD_DEFINITIONS("-DCOAL_BUILD_DIR=\"${CMAKE_BINARY_DIR}\"")
ADD_DEFINITIONS("-DCOAL_INSTALL_DIR=\"${CMAKE_INSTALL_PREFIX}\"")

SET (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH
    "The directory the headers are installed in")

add_subdirectory(ocl_load)
add_subdirectory(src)
add_subdirectory(examples)

IF (BUILD_TESTS)
    ENABLE_TESTING()
    Find_Package(Check REQUIRED)
    add_subdirectory(tests)
ENDIF (BUILD_TESTS)
