# For cross-compiling
CROSS_COMPILE=

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
INSTALL	= $(CROSS_COMPILE)install

CONF_DIR = $(PWD)

export CONF_DIR
export CROSS_COMPILE AS LD CC AR NM STRIP OBJCOPY OBJDUMP INSTALL

SUBDIRS = src test

all:
	for i in $(SUBDIRS); \
	do make -C $$i ; done

clean:
	@for i in $(SUBDIRS); \
	do make -C $$i clean; done
