# Create and install the eclipse plugin folder

if (${BUILD_OS} MATCHES "SYS_BIOS")
    install(CODE "execute_process(COMMAND make ECLIPSE_INSTALL_DIR=$ENV{DESTDIR}/../../.. WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
    message(STATUS "Placing eclipse plugin folder at $ENV{PACKAGE_ROOT_DIR}/../../..")
endif()
