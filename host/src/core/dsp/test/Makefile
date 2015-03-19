SDK = /home/al/ti/desktop-linux-sdk_01_00_00_02
CC  = g++ -O3

INCLUDES = -I. -I.. -I$(SDK)/sdk -I$(SDK)/sdk/config -I$(SDK)/sdk/pciedrv -I$(SDK)/sdk/mailBox -I$(SDK)/sdk/cmem -I$(SDK)/sdk/bufmgr -I$(SDK)/sdk/dnldmgr -I/home/al/clover/ocl_load/DLOAD_API -I/home/al/clover/ocl_load/DLOAD -I/home/al/clover/ocl_load -I/home/al/clover/include

LIBS = $(SDK)/sdk/pciedrv/lib/pciedrv.a $(SDK)/sdk/mailBox/host/lib/mailBox.a $(SDK)/sdk/dnldmgr/lib/dnldmgr.a $(SDK)/sdk/cmem/lib/cmem_drv.a $(SDK)/sdk/bufmgr/lib/bufmgr.a -lrt -lpthread -lpciaccess

.SILENT:

all: mailbox.out memory.out t.out

mailbox.out: mailbox.o driver.o 
	$(CC) mailbox.o driver.o $(LIBS) -o $@

memory.out: memory.o driver.o
	$(CC) memory.o driver.o $(LIBS) -o $@

t.out: t.o 
	$(CC) t.o -o $@

%.o:%.cpp
	$(CC) -c -fpermissive $(INCLUDES) $<

%.o:../%.cpp
	$(CC) -c -fpermissive $(INCLUDES) $<

clean:
	rm -rf *.out *.o
