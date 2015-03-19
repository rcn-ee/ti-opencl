include_directories (${Coal_SOURCE_DIR}/include)

message(STATUS "source is ${Coal_SOURCE_DIR}")
link_directories(${Coal_BINARY_DIR}/src)

add_executable(task main.cpp)

set_source_files_properties(main.cpp
        PROPERTIES COMPILE_FLAGS "-g -Wno-format")

target_link_libraries(task OpenCL)
