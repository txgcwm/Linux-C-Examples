#
#
#

SUBDIRS= kernel libusb-0.1.12 app

all:
	@set -e; \
	for i in $(SUBDIRS); \
	do $(MAKE) -C $$i ; done

clean:
	@for i in $(SUBDIRS); \
	do $(MAKE) -C $$i clean; done
