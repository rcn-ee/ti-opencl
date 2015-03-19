# If not specified, pick a default location for dependent llvm libraries
LLVM_VERSION = 33

ifeq ($(DEFAULT_DEV_INSTALL_DIR),)
   DEFAULT_DEV_INSTALL_DIR = /opt/ti
endif

ifeq ($(ARM_LLVM_DIR),)
   ARM_LLVM_DIR = $(DEFAULT_DEV_INSTALL_DIR)/llvm$(LLVM_VERSION)-install-arm
endif

ifeq ($(X86_LLVM_DIR),)
   X86_LLVM_DIR = $(DEFAULT_DEV_INSTALL_DIR)/llvm$(LLVM_VERSION)-install-x86
endif

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
  BUILD_PROCESSOR := x86
  TARGET := x86
  LLVM_DIR := $(X86_LLVM_DIR)
  CXX := g++ -m32
else ifneq (,$(findstring arm, $(UNAME_M)))
  BUILD_PROCESSOR := arm
  TARGET := arm
  LLVM_DIR := $(ARM_LLVM_DIR)
  CXX := g++
endif

LLVM_CONFIG_EXECUTABLE = $(LLVM_DIR)/bin/llvm-config
LLVM_CXXFLAGS 	= `${LLVM_CONFIG_EXECUTABLE} --cxxflags `
LLVM_LDFLAGS 	= -L $(LLVM_DIR)/lib -lpthread -lrt -ldl -lm
LLVM_LIBS	= `${LLVM_CONFIG_EXECUTABLE} --libs ${TARGET} asmparser bitwriter tablegen mcjit debuginfo interpreter irreader jit linker instrumentation ipo mcdisassembler`


# If Cross Compiling for ARM override some make variables
cross: override CXX=arm-linux-gnueabihf-g++
cross: override TARGET=arm
cross: override LLVM_CONFIG_EXECUTABLE=$(ARM_LLVM_DIR)/bin/llvm-config-host
cross: override LLVM_LDFLAGS=-L $(ARM_LLVM_DIR)/lib -lpthread -lrt -ldl -lm
# Need to explicitly add /usr/include when cross compiling to pick up
# dependent 3rd party non-system headers (like boost headers installed to 
# cross-compile host)
cross: override HOST_USR_INCLUDE=-idirafter/usr/include


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
              main.o compiler.o wga.o program.o file_manip.o options.o \
              util.o

OBJS := $(patsubst %.o, $(TARGET)/%.o, $(OBJS))

CXXFLAGS   	= ${LLVM_CXXFLAGS} -I${UTILDIR} -I${WGADIR} -I${POCLDIR} -DLLVM_3_3 \
                  ${HOST_USR_INCLUDE} -O3 -fexceptions -D_PRODUCT_VERSION=$(_PRODUCT_VERSION)
LIBS       	= ${CLANG_LIBS}  ${LLVM_LIBS}
LDFLAGS    	= ${LLVM_LDFLAGS} 

.PHONY: .FORCE 

$(EXE): ${OBJS}
	$(CXX) $^ $(LIBS) $(LDFLAGS) -o $(TARGET)/$@

cross: $(EXE)

$(TARGET)/%.o: %.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: ${UTILDIR}/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: ${WGADIR}/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: ${POCLDIR}/%.cpp | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET)/%.o: ${POCLDIR}/%.cc | $(TARGET)/
	$(CXX) $(CXXFLAGS) -c $<  -o $@

$(TARGET)/:
	mkdir -p $@

$(TARGET)/options.o: .FORCE

.FORCE:

clean: 
	rm -f x86/* arm/*
