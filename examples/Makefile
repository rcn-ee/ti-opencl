.SILENT:

ifneq ($(BUILD_OS), SYS_BIOS)
MFS  = $(wildcard */Makefile)
DIRS = $(patsubst %/Makefile,%,$(MFS))
else
MFS  = $(wildcard */Makefile.rtos)
DIRS = $(patsubst %/Makefile.rtos,%,$(MFS))
MAKEFILE = -f Makefile.rtos
endif

ifeq ($(OS), Windows_NT)
define make_in_dirs
	for %%D in ($(1)) do \
	  echo ===============  %%D  ================= & \
	  $(MAKE) $(MAKEFILE) -C %%D $(2)
endef
else  # Linux
define make_in_dirs
	for dir in $(1); do \
	  echo "=============== " $$dir " =================" ; \
	  $(MAKE) $(MAKEFILE) -C $$dir $(2); \
	done
endef
endif


all:
	$(call make_in_dirs, $(DIRS), )

test:
	$(call make_in_dirs, $(DIRS), test)

cross:
	$(call make_in_dirs, $(DIRS), cross)

clean:
	$(call make_in_dirs, $(DIRS), clean)

