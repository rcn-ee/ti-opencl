CL6X_CFLAGS  = -mo -o1 -mt -k -mw -mv6600 --abi=eabi --symdebug:none 
CLANG_CFLAGS = -cc1 -O3 -triple=spir-unknown-unknown-unknown -emit-llvm-bc -x cl -cl-std=CL1.2 -fno-builtin -ffp-contract=off -Iinclude

CL_FILES  = $(wildcard lib/*.cl lib/dsp/*.cl)
OBJECTS := ${CL_FILES:.cl=.obj}

opencl_c66_builtins.lib: $(OBJECTS)
	ar6x ru $@ $^

%.bc: %.cl
	clang $(CLANG_CFLAGS) $<

%.ll: %.bc
	llvm-dis $<

%.obj: %.bc
	cl6x $(CL6X_CFLAGS) $<

install: opencl_c66_builtins.lib
	#cp $@ <XXX>
	#cp -r include <XXX>

clean:
	@rm -rf *.obj* *.bc *.asm *.ll *.if *.opt
	@rm -rf lib/*.obj* lib/*.bc lib/*.asm lib/*.ll lib/*.if lib/*.opt
	@rm -rf lib/dsp/*.obj* lib/dsp/*.bc lib/dsp/*.asm lib/dsp/*.ll lib/dsp/*.if lib/dsp/*.opt
