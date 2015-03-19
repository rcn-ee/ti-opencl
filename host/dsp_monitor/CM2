add_custom_command(
    OUTPUT stdlib.h.embed.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/embed.py
            ${CMAKE_CURRENT_BINARY_DIR}/stdlib.h.embed.h
            ${CMAKE_CURRENT_SOURCE_DIR}/stdlib.h
            ${CMAKE_CURRENT_BINARY_DIR}/stdlib_def.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed.py
            ${CMAKE_CURRENT_SOURCE_DIR}/stdlib.h
            ${CMAKE_CURRENT_BINARY_DIR}/stdlib_def.h)

add_custom_target(generate_stdlib_h DEPENDS
                  ${CMAKE_CURRENT_BINARY_DIR}/stdlib.h.embed.h)

add_custom_command(
    OUTPUT stdlib.c.bc
    COMMAND clang -c -emit-llvm -x cl -O2 -nobuiltininc -nostdinc -fno-builtin
            ${CMAKE_CURRENT_SOURCE_DIR}/stdlib.c
            -I${CMAKE_CURRENT_BINARY_DIR}
            -o ${CMAKE_CURRENT_BINARY_DIR}/stdlib.c.bc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/stdlib.c
            ${CMAKE_CURRENT_BINARY_DIR}/stdlib_impl.h)

add_custom_command(
    OUTPUT stdlib.c.bc.embed.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/embed.py
            ${CMAKE_CURRENT_BINARY_DIR}/stdlib.c.bc.embed.h
            ${CMAKE_CURRENT_BINARY_DIR}/stdlib.c.bc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/embed.py
            ${CMAKE_CURRENT_BINARY_DIR}/stdlib.c.bc)

add_custom_target(generate_stdlib_c DEPENDS
                  ${CMAKE_CURRENT_BINARY_DIR}/stdlib.c.bc.embed.h)

add_custom_command(
    OUTPUT builtins_def.h stdlib_def.h builtins_impl.h stdlib_impl.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/builtins.py
            ${CMAKE_CURRENT_SOURCE_DIR}/builtins.def
            ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/builtins.py
            ${CMAKE_CURRENT_SOURCE_DIR}/builtins.def)

add_custom_target(generate_builtins DEPENDS
                  ${CMAKE_CURRENT_BINARY_DIR}/builtins_def.h
                  ${CMAKE_CURRENT_BINARY_DIR}/builtins_impl.h
                  ${CMAKE_CURRENT_BINARY_DIR}/stdlib_def.h
                  ${CMAKE_CURRENT_BINARY_DIR}/stdlib_impl.h)
