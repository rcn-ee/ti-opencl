EXE       = ooo_callback
CXXFLAGS = -g -O0 -fno-inline

include ../make.inc

$(EXE): ooo_callback.o
	@$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LIBS) -lrt -o $@

