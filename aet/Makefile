include ../host/Makefile.inc

ifeq ($(TI_OCL_CGT_INSTALL),)
   $(error Builtins build requires TI_OCL_CGT_INSTALL env var for C6x CGT toolset)
endif

AR=$(TI_OCL_CGT_INSTALL)/bin/ar6x
CC=$(TI_OCL_CGT_INSTALL)/bin/cl6x
CFLAGS=-o3 -mv6600 --abi=eabi --mem_model:data=far -I$(TI_OCL_CGT_INSTALL)/include -Iaet/src -Iaet/include -eoo --visibility=protected 

VPATH = aet/src

#---------------------------
# FROM SRC subdir
#---------------------------
SRCS = aet.c aet_add_trigger.c aet_aeg_manager.c aet.c aet_count_stalls.c \
          aet_data_watchpoint.c aet_data_watchpoint_range.c aet_data_watchpoint_value.c aet_evt_timer_common.c \
          aet_evt_timer_start.c aet_evt_timer_stop.c aet_evt_wm_startstop.c aet_func_profile.c aet_gen_common.c \
          aet_intr_on_stall.c aet_intr_stall_duration.c aet_program_watchpoint.c aet_program_watchpoint_range.c \
          aet_resource.c aet_timer_trigger.c aet_trace_in_data_range.c aet_trace_in_pc_range.c aet_trace_on_event.c \
          aet_trace_startstop_on_pc.c aet_trigger_builders.c aet_trigger_on_event.c iaet.c iaetParams.c

OBJS  = $(patsubst %.c,%.o,$(SRCS))

AETZIP = aet_4.18.zip

libaet.lib: $(AETZIP) $(OBJS)
libaet.lib: setup $(OBJS)
	@$(AR) r libaet.lib $(OBJS)

setup: $(AETZIP)

%.o:%.c
	@echo cl6x ... $<
	@$(CC) $(CFLAGS) $<

clean:
	rm -f *.o libaet.lib

# will move to git clone when aet repo becomes externally public
$(AETZIP):
	#wget https://gforge.ti.com/gf/download/frsrelease/1264/7695/aet_4.18.zip
	wget --no-check-certificate https://174.143.205.12/gf/download/frsrelease/1264/7695/$(AETZIP)
	unzip -a -o $(AETZIP) aet/include/* aet/src/*
	patch -p0 < aet_src.patch
	# call make again to rebuild dependencies from newly expanded files
	make -C . -j4

realclean: clean
	rm -fr aet aet*.zip

