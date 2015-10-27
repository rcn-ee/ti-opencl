# If not specified, pick a default location for dependent llvm libraries
LLVM_VERSION = 33
DEFAULT_DEV_INSTALL_DIR ?= D:/OpenCL/LLVM
ARM_LLVM_DIR ?= $(DEFAULT_DEV_INSTALL_DIR)/llvm$(LLVM_VERSION)-install-arm
X86_LLVM_DIR ?= $(DEFAULT_DEV_INSTALL_DIR)/llvm$(LLVM_VERSION)-install-x86

CLANG_LIBS 	= clangFrontendTool.lib
CLANG_LIBS 	+= clangFrontend.lib
CLANG_LIBS 	+= clangDriver.lib
CLANG_LIBS 	+= clangSerialization.lib
CLANG_LIBS 	+= clangCodeGen.lib
CLANG_LIBS 	+= clangParse.lib
CLANG_LIBS 	+= clangSema.lib
CLANG_LIBS 	+= clangEdit.lib
CLANG_LIBS 	+= clangAnalysis.lib
CLANG_LIBS 	+= clangAST.lib
CLANG_LIBS 	+= clangLex.lib
CLANG_LIBS 	+= clangBasic.lib

EXE = clocl

UNAME_M :=$(shell uname -m)

ifneq (,$(findstring 86, $(UNAME_M)))
   ifeq ($(findstring arm, $(MAKECMDGOALS)), arm) 
        CXX = arm-linux-gnueabihf-g++
        TARGET := arm
        LLVM_DIR = $(ARM_LLVM_DIR)
        LLVM_CONFIG_NAME = llvm-config-host
        HOST_USR_INCLUDE = -idirafter/usr/include
   else ifeq ($(findstring x86, $(MAKECMDGOALS)), x86) 
        CXX = cl
        TARGET := x86
        LLVM_DIR = $(X86_LLVM_DIR)
        LLVM_CONFIG_NAME = llvm-config
		HOST_USR_INCLUDE = "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include"
		HOST_OS_INCLUDE = "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include"
		HOST_BOOST = "D:\OpenCL\LLVM\boost_1_58_0"
		HOST_WIN_GETOPT = "D:\OpenCL\LLVM\getopt_mb_uni_src"
	LLVM_LDFLAGS = -m32
   else ifeq ($(findstring arm-linux, $(CXX)), arm-linux) 
        TARGET := arm
   endif
else ifneq (,$(findstring arm, $(UNAME_M)))
   TARGET := arm
   LLVM_DIR = $(ARM_LLVM_DIR)
   LLVM_CONFIG_NAME = llvm-config
endif

LLVM_CONFIG_PREFIX = $(LLVM_DIR)/bin/

LLVM_CONFIG_EXE ?= $(LLVM_CONFIG_PREFIX)$(LLVM_CONFIG_NAME)
export LLVM_CXXFLAGS = `$(LLVM_CONFIG_EXE) --cxxflags`

export CGO_LDFLAGS = `/Zi /W3 /WX- /MP /Od /Ob0 /Oy- /D WIN32 /D _WINDOWS /D _DEBUG /D _HAS_EXCEPTIONS=0 /D GTEST_HAS_RTTI=0 \
                      /D _CRT_SECURE_NO_DEPRECATE /D _CRT_SECURE_NO_WARNINGS /D _CRT_NONSTDC_NO_DEPRECATE /D _CRT_NONSTDC_NO_WARNINGS \
					  /D _SCL_SECURE_NO_DEPRECATE /D _SCL_SECURE_NO_WARNINGS /D __STDC_CONSTANT_MACROS /D __STDC_FORMAT_MACROS \
					  /D __STDC_LIMIT_MACROS /D _MBCS /Gm- /RTC1 /MDd /GS /fp:precise /Gd /TP /wd4146 /wd4180 /wd4244 /wd4258 /wd4267 \
					  /wd4291 /wd4345 /wd4351 /wd4355 /wd4456 /wd4457 /wd4458 /wd4459 /wd4503 /wd4624 /wd4722 /wd4800 \
					  /analyze- /errorReport:prompt /we4238  /EHs-c- -w14062`


# Cannot use llvm-config --ldflags because it includes -lz which is not 
# available on arago
LLVM_LDFLAGS += -L `$(LLVM_CONFIG_EXE) --libdir` -lpthread -lrt -ldl -lm
export LLVM_LIBS = `$(LLVM_CONFIG_EXE) --libs $(TARGET) asmparser bitwriter tablegen debuginfo interpreter irreader linker instrumentation ipo mcdisassembler`  #mcjit jit

UTILDIR         = ../src/core
WGADIR          = ../src/core/dsp
POCLDIR         = ../src/llvmopencl
OBJS        =  AllocasToEntry.o BarrierBlock.o BarrierTailReplication.o  \
              BreakConstantGEPs.o CanonicalizeBarriers.o DebugHelpers.o \
              Flatten.o GenerateHeader.o ImplicitLoopBarriers.o \
              ImplicitConditionalBarriers.o IsolateRegions.o \
              Kernel.o LLVMUtils.o LoopBarriers.o ParallelRegion.o \
              PHIsToAllocas.o TargetAddressSpaces.o \
              VariableUniformityAnalysis.o Workgroup.o \
              WorkItemAliasAnalysis.o WorkitemHandler.o \
              WorkitemHandlerChooser.o WorkitemLoops.o WorkitemReplication.o\
              SimplifyShuffleBIFCall.o \
              main.o compiler.o wga.o program.o file_manip.o options.o \
              util.o
#WIVectorize.o			  

OBJS := $(patsubst %.o, $(TARGET)/%.o, $(OBJS))

CXXFLAGS := $(LLVM_CXXFLAGS) -I$(UTILDIR) -I$(WGADIR) -I$(POCLDIR) -I$(HOST_WIN_GETOPT)\
            -I$(HOST_USR_INCLUDE) -I$(HOST_OS_INCLUDE) -I$(HOST_BOOST) -fexceptions -std=c++11 \
            -D_PRODUCT_VERSION=$(_PRODUCT_VERSION) $(CXXFLAGS) $(CGO_LDFLAGS) /EHs -D_MSC_VER=1800

LIBS      = $(CLANG_LIBS)  $(LLVM_LIBS)
LDFLAGS  := $(LLVM_LDFLAGS) $(LDFLAGS)

.PHONY: .FORCE 

$(TARGET): $(TARGET)/.touch $(EXE) 

$(EXE): $(OBJS)
	$(CXX) /out:$(EXE).exe /link$^ $(LIBS) $(LDFLAGS) /Fo$(TARGET)/$@

$(TARGET)/%.o: %.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< /Fo$@

$(TARGET)/%.o: $(UTILDIR)/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< /Fo$@

$(TARGET)/%.o: $(WGADIR)/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< /Fo$@

$(TARGET)/%.o: $(POCLDIR)/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< /Fo$@

$(TARGET)/%.o: $(POCLDIR)/%.cc | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $<  /Fo$@
	


$(TARGET)/.touch: 
	mkdir -p $(TARGET); touch $(TARGET)/.touch

$(TARGET)/options.o: .FORCE

.FORCE:

clean: 
	rm -f x86/* arm/*
