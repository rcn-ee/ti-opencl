if (CROSS_COMPILE)
   include_directories( ${CMAKE_FIND_ROOT_PATH} ${HOST_USR_INCLUDE_PATH} )
endif()

include_directories( ${PROJECT_SOURCE_DIR}/src/core/dsp )

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c++11 -D_FILE_OFFSET_BITS=64 -DBOOST_SYSTEM_NO_DEPRECATED=1 -DBOOST_SYSTEM_NO_LIB=1") 
SET(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (DAEMON_SRC mctd.cpp ../src/core/error_report.cpp)

if (K2X_BUILD OR K2G_BUILD)
    list(APPEND DAEMON_SRC mpm_load.cpp)
endif()

add_executable(ti-mctd ${DAEMON_SRC})
add_executable(ti-mct-heap-check heap_check.cpp)

find_library(CMEM_LIB          ticmem)
find_library(JSON_LIB          json-c)

if (K2X_BUILD OR K2G_BUILD)
    find_library(MPMCLIENT_LIB     mpmclient)
    target_link_libraries(ti-mctd ${MPMCLIENT_LIB})
endif()

target_link_libraries(ti-mctd ${CMEM_LIB} ${JSON_LIB} pthread rt)
target_link_libraries(ti-mct-heap-check pthread rt)

install(TARGETS ti-mctd RUNTIME DESTINATION /usr/bin ${OCL_BPERMS})
install(TARGETS ti-mct-heap-check RUNTIME DESTINATION /usr/bin ${OCL_BPERMS})

install(DIRECTORY DESTINATION /lib/systemd/system ${OCL_DPERMS})
install(DIRECTORY DESTINATION /etc/ti-mctd ${OCL_DPERMS})
if (K2X_BUILD OR K2G_BUILD)
    install(FILES ti-mct-daemon.service.k2x DESTINATION  /lib/systemd/system RENAME ti-mct-daemon.service  ${OCL_FPERMS})
    if (K2E_BUILD OR K2G_BUILD)
        install(FILES ti_mctd_config.json.k2eg DESTINATION  /etc/ti-mctd RENAME ti_mctd_config.json  ${OCL_FPERMS})
    elseif (K2L_BUILD)
        install(FILES ti_mctd_config.json.k2l DESTINATION  /etc/ti-mctd RENAME ti_mctd_config.json  ${OCL_FPERMS})
    else ()
        install(FILES ti_mctd_config.json.k2h DESTINATION  /etc/ti-mctd RENAME ti_mctd_config.json  ${OCL_FPERMS})
    endif ()
elseif (AM57_BUILD)
    install(FILES ti-mct-daemon.service.am57x DESTINATION  /lib/systemd/system RENAME ti-mct-daemon.service  ${OCL_FPERMS})
    install(FILES ti_mctd_config.json.am57x DESTINATION  /etc/ti-mctd RENAME ti_mctd_config.json  ${OCL_FPERMS})
endif()
