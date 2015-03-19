# Detect CLANG
if (NOT LLVM_INCLUDE_DIR OR NOT LLVM_LIB_DIR)
   message(FATAL_ERROR "No LLVM, Clang support requires LLVM")
else (NOT LLVM_INCLUDE_DIR OR NOT LLVM_LIB_DIR)

MACRO(FIND_AND_ADD_CLANG_LIB _libname_)
find_library(CLANG_${_libname_}_LIB NAMES ${_libname_} PATHS ${LLVM_LIB_DIR} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if (CLANG_${_libname_}_LIB)
   set(CLANG_LIBS ${CLANG_LIBS} ${CLANG_${_libname_}_LIB})
endif(CLANG_${_libname_}_LIB)
ENDMACRO(FIND_AND_ADD_CLANG_LIB)

set(CLANG_INCLUDE_DIRS ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIR})


FIND_AND_ADD_CLANG_LIB(clangFrontendTool)
FIND_AND_ADD_CLANG_LIB(clangFrontend)
FIND_AND_ADD_CLANG_LIB(clangDriver)
FIND_AND_ADD_CLANG_LIB(clangSerialization)
FIND_AND_ADD_CLANG_LIB(clangCodeGen)
FIND_AND_ADD_CLANG_LIB(clangParse)
FIND_AND_ADD_CLANG_LIB(clangSema)
FIND_AND_ADD_CLANG_LIB(clangEdit)
FIND_AND_ADD_CLANG_LIB(clangAnalysis)
#FIND_AND_ADD_CLANG_LIB(clangIndex)   Removed in 3.1
#FIND_AND_ADD_CLANG_LIB(clangRewrite) Now clangRewriteCore,clangRewriteFrontend
FIND_AND_ADD_CLANG_LIB(clangAST)
FIND_AND_ADD_CLANG_LIB(clangLex)
FIND_AND_ADD_CLANG_LIB(clangBasic)

MESSAGE(STATUS "Clang libs: " ${CLANG_LIBS})

# We build using a specific version of llvm/clang so point to it instead
# of using any installed version (see FindLLVM.cmake)
if (HAWKING_CROSS_COMPILE OR C6678_BUILD)
  set (CLANG_INSTALL_DIR ${X86_LLVM_DIR})
else()
  set (CLANG_INSTALL_DIR ${ARM_LLVM_DIR})
endif()

find_program(CLANG_EXECUTABLE
  NAMES clang
  PATHS
  ${CLANG_INSTALL_DIR}/bin
  NO_DEFAULT_PATH
)

if(CLANG_EXECUTABLE)
  set(CLANG_FOUND TRUE)
endif(CLANG_EXECUTABLE)

if(CLANG_FOUND)
  message(STATUS "Found Clang: ${CLANG_EXECUTABLE}")
else(CLANG_FOUND)
  if(CLANG_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Clang")
  endif(CLANG_FIND_REQUIRED)
endif(CLANG_FOUND)


endif (NOT LLVM_INCLUDE_DIR OR NOT LLVM_LIB_DIR)
