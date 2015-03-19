include_directories (.
                     ${Coal_SOURCE_DIR}/include
                     ${Coal_SOURCE_DIR}/examples/testsum)

message(STATUS "source is ${Coal_SOURCE_DIR}")
link_directories(${Coal_BINARY_DIR}/src)

add_executable(testsum main.cpp)

set_source_files_properties(main.cpp
        PROPERTIES COMPILE_FLAGS "-g -Wno-format")

target_link_libraries(testsum OpenCL)
