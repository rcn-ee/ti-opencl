.SILENT:

MFS  = $(wildcard */Makefile)
DIRS = $(patsubst %/Makefile,%,$(MFS))

all:
	for dir in $(DIRS); do \
	  echo "=============== " $$dir " =================" ; \
	  $(MAKE) -C $$dir; \
	done

test:
	for dir in $(DIRS); do \
	  echo "=============== " $$dir " =================" ; \
          $(MAKE) -C $$dir test; \
        done

cross:
	for dir in $(DIRS); do \
	  echo "=============== " $$dir " =================" ; \
          $(MAKE) -C $$dir cross; \
        done

clean:
	for dir in $(DIRS); do \
	  $(MAKE) -C $$dir clean; \
	done

