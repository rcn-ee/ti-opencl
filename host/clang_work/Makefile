TI_CFLAGS    = -mt -k -o3 -os -mw -mv6600 --abi=eabi --z --symdebug:none --gen_pic 
CLANG_CFLAGS = -cc1 -O3 -triple i386-unknown-linux-gnu -target-cpu pentium4 -emit-llvm-bc -x cl 

OBJS = simple.obj
OUT = simple.out

$(OUT):

%.out: %.bc
	cl6x --v --f --use_llvm $(TI_CFLAGS) $< -z  -cr rts6600_elf_pic.lib --dynamic=exe --relocatable --single_data_segment --no_entry_point -m opencl.map -o opencl.out

%.obj: %.c
	cl6x $(TI_CFLAGS) $<
	
%.bc: %.cl
	clang $(CLANG_CFLAGS) $<

%.ll: %.bc
	llvm-dis $<

%.obj: %.bc
	cl6x --use_llvm $(TI_CFLAGS) $<

clean:
	rm -rf *.obj* *.bc *.out stats.txt *.asm *.ll *.if *.opt
