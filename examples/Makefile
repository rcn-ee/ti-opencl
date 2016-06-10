.SILENT:

ifneq ($(BUILD_OS), SYS_BIOS)
MFS  = $(wildcard */Makefile)
DIRS = $(patsubst %/Makefile,%,$(MFS))
else
MFS  = $(wildcard */Makefile.rtos)
DIRS = $(patsubst %/Makefile.rtos,%,$(MFS))
MAKEFILE = -f Makefile.rtos
endif

all:
	for dir in $(DIRS); do \
	  echo "=============== " $$dir " =================" ; \
	  $(MAKE) $(MAKEFILE) -C $$dir; \
	done

test:
	for dir in $(DIRS); do \
	  echo "=============== " $$dir " =================" ; \
          $(MAKE) $(MAKEFILE) -C $$dir test; \
        done

cross:
	for dir in $(DIRS); do \
	  echo "=============== " $$dir " =================" ; \
          $(MAKE) -C $$dir cross; \
        done

clean:
	for dir in $(DIRS); do \
	  $(MAKE) $(MAKEFILE) -C $$dir clean; \
	done

