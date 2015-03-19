TARGS = stdlib.h.embed.h stdlib.c.bc stdlib.c.bc.embed.h builtins_def.h stdlib_def.h builtins_impl.h stdlib_impl.h

all : $(TARGS)

clean :
	rm -rf $(TARGS)

stdlib.h.embed.h : embed.py stdlib.h stdlib_def.h
	embed.py stdlib.h.embed.h stdlib.h stdlib_def.h

stdlib.c.bc : stdlib.c stdlib_impl.h
	clang -c -emit-llvm -x cl -O2 -nobuiltininc -nostdinc -fno-builtin stdlib.c -I. -o stdlib.c.bc

stdlib.c.bc.embed.h : embed.py stdlib.c.bc
	embed.py stdlib.c.bc.embed.h stdlib.c.bc

builtins_def.h stdlib_def.h builtins_impl.h stdlib_impl.h : builtins.py builtins.def
	builtins.py builtins.def ./
