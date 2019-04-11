# libocl_util.a

include_directories( ${CMAKE_SOURCE_DIR}/include )

if (CROSS_COMPILE)
   include_directories( ${CMAKE_FIND_ROOT_PATH})
endif()

if ((${BUILD_OS} MATCHES "SYS_BIOS"))
    include_directories (${BIOS_USR_INCLUDE_PATH})
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -O3 -fPIC -fpermissive")

set(util_src report_timing.cpp decode_error.cpp read_binary.cpp)

add_library(ocl_util STATIC ${util_src})

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

install(TARGETS ocl_util DESTINATION /usr/lib ${OCL_FPERMS})
install(FILES ocl_util.h DESTINATION /usr/include ${OCL_FPERMS})
