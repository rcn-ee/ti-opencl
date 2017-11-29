EXE       = custom_device_api_test
CPP_FLAGS = -O3

include ../make.inc

$(EXE): main.o  $(EXE_MD)
	@$(CPP) $(CPP_FLAGS) main.o $(LD_FLAGS) $(LIBS) -o $@

clean::
	@rm -fr $(EXE_MD)
