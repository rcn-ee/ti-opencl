include ../Makefile.inc

CLANG_LIBS 	= -lclangFrontendTool
CLANG_LIBS 	+= -lclangFrontend
CLANG_LIBS 	+= -lclangDriver
CLANG_LIBS 	+= -lclangSerialization
CLANG_LIBS 	+= -lclangCodeGen
CLANG_LIBS 	+= -lclangParse
CLANG_LIBS 	+= -lclangSema
CLANG_LIBS 	+= -lclangEdit
CLANG_LIBS 	+= -lclangAnalysis
CLANG_LIBS 	+= -lclangAST
CLANG_LIBS 	+= -lclangLex
CLANG_LIBS 	+= -lclangBasic

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
        CXX = $(NATIVE_GXX)
        TARGET := x86
        LLVM_DIR = $(X86_LLVM_DIR)
        LLVM_CONFIG_NAME = llvm-config
	LLVM_LDFLAGS = -ltinfo
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
LLVM_CXXFLAGS = `$(LLVM_CONFIG_EXE) --cxxflags`

ifneq ($(MAKECMDGOALS), clean)
# -lz is not available on arago
LLVM_LDFLAGS += $(subst -lz,,$(shell $(LLVM_CONFIG_EXE) --ldflags))
LLVM_LDFLAGS += -lpthread -lrt -ldl -lm
LLVM_LIBS = `$(LLVM_CONFIG_EXE) --libs`
endif

POCLDIR         = ./llvmopencl
OBJS        = AllocasToEntry.o BarrierBlock.o BarrierTailReplication.o \
              BreakConstantGEPs.o CanonicalizeBarriers.o DebugHelpers.o \
              Flatten.o GenerateHeader.o IsolateRegions.o \
              Kernel.o LLVMUtils.o LoopBarriers.o ParallelRegion.o \
              PHIsToAllocas.o TargetAddressSpaces.o \
              VariableUniformityAnalysis.o Workgroup.o \
              WorkItemAliasAnalysis.o WorkitemHandler.o \
              WorkitemHandlerChooser.o WorkitemLoops.o \
              SimplifyShuffleBIFCall.o PrivatizationAliasAnalysis.o \
              main.o compiler.o wga.o program.o file_manip.o options.o \
              llvm_util.o

OBJS := $(patsubst %.o, $(TARGET)/%.o, $(OBJS))

CXXFLAGS := $(LLVM_CXXFLAGS) -I$(POCLDIR) \
            $(HOST_USR_INCLUDE) -O3 -fexceptions -std=c++0x \
	    -DBOOST_SYSTEM_NO_DEPRECATED=1 \
	    -DBOOST_SYSTEM_NO_LIB=1 \
            -D_PRODUCT_VERSION=$(_PRODUCT_VERSION) $(CXXFLAGS) 

CXXFLAGS += -D_BUILD_ID=$(GIT_SHA)

LIBS      = $(CLANG_LIBS)  $(LLVM_LIBS)
LDFLAGS  := $(LLVM_LDFLAGS) $(LDFLAGS)
STATIC    = -static

.PHONY: .FORCE 

$(TARGET): $(TARGET)/.touch $(TARGET)/$(EXE)

$(TARGET)/$(EXE): $(OBJS)
	$(CXX) $^ $(STATIC) $(LIBS) $(LDFLAGS) -o $@

$(TARGET)/%.o: %.cpp | $(TARGET)/.touch
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: $(POCLDIR)/%.cpp | $(TARGET)/.touch
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: $(POCLDIR)/%.cc | $(TARGET)/.touch
	$(CXX) $(CXXFLAGS) -c $<  -o $@

$(TARGET)/.touch: 
	mkdir -p $(TARGET); touch $(TARGET)/.touch

$(TARGET)/options.o: .FORCE

.FORCE:

clean: 
	rm -f x86/* arm/*
