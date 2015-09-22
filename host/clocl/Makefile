# If not specified, pick a default location for dependent llvm libraries
LLVM_VERSION = 36
DEFAULT_DEV_INSTALL_DIR ?= /opt/ti
ARM_LLVM_DIR ?= $(DEFAULT_DEV_INSTALL_DIR)/llvm$(LLVM_VERSION)-install-arm
X86_LLVM_DIR ?= $(DEFAULT_DEV_INSTALL_DIR)/llvm$(LLVM_VERSION)-install-x86

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
        TARGET := x86
        LLVM_DIR = $(X86_LLVM_DIR)
        LLVM_CONFIG_NAME = llvm-config
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

# -lz is not available on arago
LLVM_LDFLAGS = $(subst -lz,,$(shell $(LLVM_CONFIG_EXE) --ldflags))
LLVM_LIBS = `$(LLVM_CONFIG_EXE) --libs $(TARGET) asmparser bitwriter tablegen mcjit debuginfo interpreter irreader jit linker instrumentation ipo mcdisassembler`

UTILDIR         = ../src/core
WGADIR          = ../src/core/dsp
POCLDIR         = ../src/llvmopencl
OBJS        = AllocasToEntry.o BarrierBlock.o BarrierTailReplication.o \
              BreakConstantGEPs.o CanonicalizeBarriers.o DebugHelpers.o \
              Flatten.o GenerateHeader.o ImplicitLoopBarriers.o \
              ImplicitConditionalBarriers.o IsolateRegions.o \
              Kernel.o LLVMUtils.o LoopBarriers.o ParallelRegion.o \
              PHIsToAllocas.o TargetAddressSpaces.o \
              VariableUniformityAnalysis.o WIVectorize.o Workgroup.o \
              WorkItemAliasAnalysis.o WorkitemHandler.o \
              WorkitemHandlerChooser.o WorkitemLoops.o WorkitemReplication.o\
              SimplifyShuffleBIFCall.o \
              main.o compiler.o wga.o program.o file_manip.o options.o \
              util.o

OBJS := $(patsubst %.o, $(TARGET)/%.o, $(OBJS))

CXXFLAGS := $(LLVM_CXXFLAGS) -I$(UTILDIR) -I$(WGADIR) -I$(POCLDIR) \
            -$(HOST_USR_INCLUDE) -O3 -fexceptions -std=c++0x \
            -D_PRODUCT_VERSION=$(_PRODUCT_VERSION) $(CXXFLAGS) 

LIBS      = $(CLANG_LIBS)  $(LLVM_LIBS)
LDFLAGS  := $(LLVM_LDFLAGS) $(LDFLAGS)

.PHONY: .FORCE 

$(TARGET): $(TARGET)/.touch $(EXE) 

$(EXE): $(OBJS)
	$(CXX) $^ $(LIBS) $(LDFLAGS) -o $(TARGET)/$@

$(TARGET)/%.o: %.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: $(UTILDIR)/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: $(WGADIR)/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: $(POCLDIR)/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: $(POCLDIR)/%.cc | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $<  -o $@

$(TARGET)/.touch: 
	mkdir -p $(TARGET); touch $(TARGET)/.touch

$(TARGET)/options.o: .FORCE

.FORCE:

clean: 
	rm -f x86/* arm/*
