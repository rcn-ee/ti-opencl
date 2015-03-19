# Compiler name (with path, if not in path)
CC=gcc -m32

LIBS=${SDK}/sdk/dnldmgr/lib/dnldmgr.a \
     ${SDK}/sdk/pciedrv/lib/pciedrv.a \
     ${SDK}/sdk/cmem/lib/cmem_drv.a \
     ${SDK}/sdk/sync/lib/sync.a 

# Mainly used for include paths
INCLUDES=-I ${SDK}/sdk\
	-I ${SDK}/sdk/pciedrv\
	-I ${SDK}/sdk/dnldmgr\
	-I inc\
	-I ${SDK}/sdk/cmem

LDFLAGS=-lpciaccess -lpthread -lbfd 

SOURCES= init_global_shared_mem.c
OBJECTS= init_global_shared_mem.o

init_global_shared_mem: $(OBJECTS) $(LIBS)
	$(CC)  $(OBJECTS) $(LIBS) -static -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(INCLUDES) $(DEFINES) -c $< -o $@

clean:
	@rm -fr *.o init_global_shared_mem

