CC = cl6x -g -mv6600 --abi=eabi

INCLUDES= -i $(SDK)/sdk/pciedrv \
	  -i $(SDK)/sdk/mailBox \
	  -i $(SDK)/sdk/mailBox/inc \
	  -i $(SDK)/sdk/buffmgr \
	  -i $(SDK)/sdk/config \
	  -i $(PDK)/packages \
	  -i ../inc \
	  -i ../src/core/dsp \
	  -D MAILBOX_ON_HOST

LIBS   = -i$(PDK)/packages/ti/platform/evmc6678l/platform_lib/lib/debug 
LIBS  += -i$(SDK)/sdk/mailBox/c66x/lib
LIBS  += -lti.platform.evm6678l.lite.lib
LIBS  += -lmailBox.a

SOURCES =monitor.c dsp_rpc.asm
OBJS1  =$(patsubst %.c,%.obj,$(SOURCES))
OBJS  =$(patsubst %.asm,%.obj,$(OBJS1))

HEX = monitor.hex
EXE = monitor.out

$(HEX): $(EXE)
	hex6x --issue_remarks -i -memwidth=8 -romwidth=8 -o $@ $<

$(EXE): $(OBJS) monitor.cmd
	@mkdir -p $(dir $@)
	$(CC) -z -w -mmonitor.map --rom_model $^ $(LIBS) -o $@ 

%.obj : %.c
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDES) -c $< 

%.obj : %.asm
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDES) -c $< 

clean:
	@rm -fr $(HEX) $(EXE) $(OBJS) *.map
