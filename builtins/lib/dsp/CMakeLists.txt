FILE (GLOB FILES         "*.cl")
FILE (GLOB FILES_GENERIC "../*.cl")

LIST (APPEND FILES ${FILES_GENERIC})

#set (FILES math misc nextafter)

FOREACH (filename ${FILES})

GET_FILENAME_COMPONENT(base ${filename} NAME_WE)
GET_FILENAME_COMPONENT(path ${filename} PATH)
set (file ${path}/${base})

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${base}.bc
    COMMAND ${CLANG_EXECUTABLE} -cc1 -O3 -triple spir-unknown-unknown-unknown
                  -emit-llvm-bc -x cl -fno-builtin -ffp-contract=off
                  -I${CMAKE_CURRENT_SOURCE_DIR}/../../../include/clc
                  ${file}.cl
                  -o ${CMAKE_CURRENT_BINARY_DIR}/${base}.bc
    DEPENDS ${file}.cl)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${base}.obj
    COMMAND cl6x --f --use_llvm -mo -o1 -mt -k -mw -mv6600 --abi=eabi --symdebug:none
            ${CMAKE_CURRENT_BINARY_DIR}/${base}.bc
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${base}.bc)

LIST (APPEND OBJS ${base}.obj)

ENDFOREACH()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/opencl_c66_builtins.lib
    COMMAND ar6x ru opencl_c66_builtins.lib ${OBJS}
    DEPENDS ${OBJS})

add_custom_target(generate_dsp_builtins DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/opencl_c66_builtins.lib)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/opencl_c66_builtins.lib 
        DESTINATION lib 
	${OCL_FPERMS})
