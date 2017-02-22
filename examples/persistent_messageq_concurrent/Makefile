EXE = persistent_messageq_concurrent
include ../make.inc

$(EXE):
	@$(MAKE) -C dsp kernel.dsp_h
	@$(MAKE) -C host

clean::
	@$(MAKE) -C dsp clean
	@$(MAKE) -C host clean

