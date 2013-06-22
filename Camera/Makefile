TOP_DIR := $(shell pwd)

export TOP_DIR

SUBDIRS = x264 src

all:
	for i in $(SUBDIRS); \
	do make -C $$i ; done

clean:
	for i in $(SUBDIRS); \
	do make -C $$i clean; done
