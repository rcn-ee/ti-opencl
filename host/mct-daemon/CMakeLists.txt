if (CROSS_COMPILE)
   include_directories( ${CMAKE_FIND_ROOT_PATH} ${HOST_USR_INCLUDE_PATH} )
endif()

include_directories( ${PROJECT_SOURCE_DIR}/src/core/dsp )

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c++11 -D_FILE_OFFSET_BITS=64 -DBOOST_SYSTEM_NO_DEPRECATED=1")

SET(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(mctd mctd.cpp)
add_executable(opencl-heap-check opencl-heap-check.cpp)

find_library(CMEM_LIB          ticmem)

target_link_libraries(mctd ${CMEM_LIB})
target_link_libraries(opencl-heap-check pthread rt)

install(TARGETS mctd RUNTIME DESTINATION /usr/bin ${OCL_BPERMS})
install(TARGETS opencl-heap-check RUNTIME DESTINATION /usr/bin ${OCL_BPERMS})
