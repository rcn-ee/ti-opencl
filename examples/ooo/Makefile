EXE       = ooo
CXXFLAGS = -g -O0 -fno-inline

include ../make.inc

$(EXE): ooo.o
	@$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LIBS) -lrt -o $@
