EXE         = ../persistent_messageq_concurrent
CXXFLAGS   = -O2 -I../../persistent_common

VPATH = ../../persistent_common

include ../../make.inc

$(EXE): host.o host_assist.o
	@echo Linking $(EXE)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LIBS) -ltiipc -o $@

host.o: ../dsp/kernel.dsp_h msgq.h
