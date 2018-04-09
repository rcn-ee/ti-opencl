EXE = tidl

RM = rm
AR = ar

HOST_DIR = tidl_wrapper
HOST_LIB_NAME = tidl_wrapper.a
HOST_LIB = $(HOST_DIR)/$(HOST_LIB_NAME)

DSP_OUTFILE = dsp/ocl_wrapper.out

include ../make.inc

ifeq ($(BUILD), debug)
	CXXFLAGS += -Og -g -ggdb
else
	CXXFLAGS += -O3
endif

CXXFLAGS += -I. -I$(HOST_DIR)/inc -std=c++11

HEADERS = 
LIBS    = -lOpenCL -locl_util -lpthread

tidl: SOURCES = main.cpp multiple_executors.cpp

$(EXE): $(HOST_LIB) $(HEADERS) $(DSP_OUTFILE) $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(HOST_LIB) $(LDFLAGS) $(LIBS) -o $@

.PHONY: $(DSP_OUTFILE)
$(DSP_OUTFILE):
	$(MAKE) -C dsp

.PHONY: $(HOST_LIB)
$(HOST_LIB):
	$(MAKE) -C $(HOST_DIR) $(HOST_LIB_NAME)

clean::
	$(RM) -f $(EXE) stats_tool_out.*
	$(MAKE) -C $(HOST_DIR) clean
	$(MAKE) -C dsp clean
