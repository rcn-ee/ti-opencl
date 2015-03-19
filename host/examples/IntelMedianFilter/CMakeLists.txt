include_directories (${Coal_SOURCE_DIR}/include)

message(STATUS "source is ${Coal_SOURCE_DIR}")
link_directories(${Coal_BINARY_DIR}/src)

add_executable(median MedianFilter.cpp utils.cpp)

set_source_files_properties(MedianFilter.cpp
        PROPERTIES COMPILE_FLAGS "-g -Wno-format")

set_source_files_properties(utils.cpp
        PROPERTIES COMPILE_FLAGS "-g -fpermissive -Wno-format")

target_link_libraries(median OpenCL)
