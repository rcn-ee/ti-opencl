.SILENT:

ifneq ($(BUILD_OS), SYS_BIOS)
    # Persistent examples
    P_MFS  = $(wildcard persistent*/Makefile)
    P_DIRS = $(patsubst %/Makefile,%,$(P_MFS))

    # All other examples
    NP_MFS  = $(wildcard */Makefile)
    NP_DIRS = $(patsubst %/Makefile,%,$(NP_MFS))
    NP_DIRS := $(filter-out $(P_DIRS),$(NP_DIRS))

    ifeq ($(EXAMPLE_SET), PERSISTENT_ONLY)
    DIRS = $(P_DIRS)
    else ifeq ($(EXAMPLE_SET), EXCLUDE_PERSISTENT)
    DIRS = $(NP_DIRS)
    else
    DIRS = $(NP_DIRS) $(P_DIRS)
    endif
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

.PHONY: test
test:
	$(call make_in_dirs, $(DIRS), test)

.PHONY: cross
cross: all

.PHONY: clean
clean:
	$(call make_in_dirs, $(DIRS), clean)

