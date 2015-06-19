# If not specified, pick a default location for TI dependent packages
LLVM_MAJOR_VERSION = 3
LLVM_MINOR_VERSION = 3

ifeq ($(DEFAULT_DEV_INSTALL_DIR),)
   DEFAULT_DEV_INSTALL_DIR = /opt
endif

ifeq ($(ARM_LLVM_DIR),)
   ARM_LLVM_DIR = $(DEFAULT_DEV_INSTALL_DIR)/ti-llvm-$(LLVM_MAJOR_VERSION).$(LLVM_MINOR_VERSION)
endif

ifeq ($(X86_LLVM_DIR),)
   X86_LLVM_DIR = $(DEFAULT_DEV_INSTALL_DIR)/ti/llvm$(LLVM_MAJOR_VERSION)$(LLVM_MINOR_VERSION)-install-x86
endif


UNAME_M :=$(shell uname -m)
ifneq (,$(findstring 86, $(UNAME_M)))
  ifeq ($(TI_OCL_CGT_INSTALL),)
    $(error Builtins build requires TI_OCL_CGT_INSTALL env var for C6x CGT toolset)
  endif
  CLANG := $(X86_LLVM_DIR)/bin/clang
else ifneq (,$(findstring arm, $(UNAME_M)))
  CLANG := $(ARM_LLVM_DIR)/bin/clang
endif

#CL6X    = $(TI_OCL_CGT_INSTALL)/bin/cl6x
CL6X    = cl6x

CL6X_CFLAGS   = -q -o3 -mv6600 --abi=eabi --use_g3 -mt -mo 
CL6X_CFLAGS  += --mem_model:code=far --mem_model:data=far
CLANG_CFLAGS  = -cc1 -O2 -triple=spir-unknown-unknown-unknown -Fvisibility=protected
CLANG_CFLAGS += -x cl -cl-std=CL1.2 -fno-builtin -ffp-contract=off 
CLANG_CFLAGS += -Iinclude 

CL_FILES = $(wildcard lib/*.cl lib/dsp/*.cl)
OBJECTS := ${CL_FILES:.cl=.obj}
OBJECTS := $(subst lib/dsp/,, $(OBJECTS))
OBJECTS := $(subst lib/,, $(OBJECTS))

VPATH    = lib:lib/dsp

all: dsp.lib 

dsp.lib: $(OBJECTS)
	@echo $@ Archiving
	@ar6x qru $@ $^

pch: 
	mkdir -p usr/share/ti/opencl
	cp include/clc.h usr/share/ti/opencl
	$(CLANG) -cc1 -v -O2 -triple=c6000-unknown-unknown-unknown -Fvisibility=protected -x cl -fno-builtin -ffp-contract=off -fno-trigraphs --relocatable-pch -emit-pch -isysroot ${CURDIR} usr/share/ti/opencl/clc.h -o clc.h.pch 

#------------------------------------------------------------------------
# Uncomment to use clocl to compile instead of clang + cl6x direct
#------------------------------------------------------------------------
CLOCL        = clocl
CLOCL_CFLAGS = -l -b
%.obj: %.cl
	@echo $< Clocl
	$(CLOCL) $(CLOCL_CFLAGS)  $< 

#%.bc: %.cl
#	@echo $< Parsing
#	@$(CLANG) $(CLANG_CFLAGS) -emit-llvm-bc $< -o $@

#%.obj: %.bc
#	@echo $< Generating
#	@$(CL6X) $(CL6X_CFLAGS) $<

clean:
	@rm -rf *.obj* *.bc *.asm *.ll *.if *.opt *.lib *.map *.out include/clc.h.pch *.h
