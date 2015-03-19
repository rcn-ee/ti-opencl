INCLUDE_DIRECTORIES(${Coal_SOURCE_DIR}/include ${CHECK_INCLUDE_DIRS}
${PROJECT_SOURCE_DIR}/include)
LINK_DIRECTORIES(${Coal_BINARY_DIR}/src ${CHECK_LIBRARY_DIRS})

set(OPENCL_TESTS_SOURCE
    tests.c
    test_platform.cpp
    test_device.cpp
    test_context.cpp
    test_commandqueue.cpp
    test_mem.cpp
    test_kernel.cpp
    test_program.cpp
    test_builtins.cpp
    )

add_executable(tests ${OPENCL_TESTS_SOURCE})
target_link_libraries(tests OpenCL ${CHECK_LIBRARIES})

MACRO(OPENCL_TEST EXECUTABLE_NAME TEST_NAME)
    add_test(${TEST_NAME} ${EXECUTABLE_NAME} ${TEST_NAME})
ENDMACRO(OPENCL_TEST)

OPENCL_TEST(tests platform)
OPENCL_TEST(tests device)
OPENCL_TEST(tests context)
OPENCL_TEST(tests commandqueue)
OPENCL_TEST(tests mem)
OPENCL_TEST(tests kernel)
OPENCL_TEST(tests program)
OPENCL_TEST(tests builtins)
