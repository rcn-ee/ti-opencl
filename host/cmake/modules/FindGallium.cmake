# Detect Gallium
if (NOT GALLIUM_DIR)
   message(FATAL_ERROR "Define GALLIUM_DIR to build Coal")
else (NOT GALLIUM_DIR)

MACRO(FIND_AND_ADD_GALLIUM_LIB _libname_)
find_library(GALLIUM_${_libname_}_LIB ${_libname_} ${GALLIUM_DIR}//src/gallium/auxiliary/${_libname_})
if (GALLIUM_${_libname_}_LIB)
   set(GALLIUM_LIBS ${GALLIUM_LIBS} ${GALLIUM_${_libname_}_LIB})
endif(GALLIUM_${_libname_}_LIB)
ENDMACRO(FIND_AND_ADD_GALLIUM_LIB)

set(GALLIUM_INCLUDE_DIRS ${GALLIUM_INCLUDE_DIRS} ${GALLIUM_DIR}/src/gallium/include)
set(GALLIUM_INCLUDE_DIRS ${GALLIUM_INCLUDE_DIRS} ${GALLIUM_DIR}/src/gallium/auxiliary)
set(GALLIUM_INCLUDE_DIRS ${GALLIUM_INCLUDE_DIRS} ${GALLIUM_DIR}/src/gallium/drivers)

#FIND_AND_ADD_GALLIUM_LIB(pipebuffer)
#FIND_AND_ADD_GALLIUM_LIB(sct)
#FIND_AND_ADD_GALLIUM_LIB(draw)
#FIND_AND_ADD_GALLIUM_LIB(rtasm)
#FIND_AND_ADD_GALLIUM_LIB(translate)
#FIND_AND_ADD_GALLIUM_LIB(cso_cache)
#FIND_AND_ADD_GALLIUM_LIB(tgsi)

#FIXME: always some other libutil is found
#FIND_AND_ADD_GALLIUM_LIB(util)
#set(GALLIUM_LIBS ${GALLIUM_LIBS} ${GALLIUM_DIR}//src/gallium/auxiliary/util/libutil.a)

MESSAGE(STATUS "Gallium libs: " ${GALLIUM_LIBS})

if(GALLIUM_LIBS)
  set(GALLIUM_FOUND TRUE)
endif(GALLIUM_LIBS)

if(GALLIUM_FOUND)
  message(STATUS "Found Gallium")
else(GALLIUM_FOUND)
  if(GALLIUM_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Gallium")
  endif(GALLIUM_FIND_REQUIRED)
endif(GALLIUM_FOUND)

endif (NOT GALLIUM_DIR)
