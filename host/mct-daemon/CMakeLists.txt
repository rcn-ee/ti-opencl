if (CROSS_COMPILE)
   include_directories( ${CMAKE_FIND_ROOT_PATH} ${HOST_USR_INCLUDE_PATH} )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c++11 -D_FILE_OFFSET_BITS=64")

SET(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(mctd mctd.cpp)

find_library(CMEM_LIB          ticmem)
target_link_libraries(mctd ${CMEM_LIB})

install(TARGETS mctd RUNTIME DESTINATION /usr/bin ${OCL_BPERMS})
