#
#
#

EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -I$(LDDINC)

ifneq ($(KERNELRELEASE),)

obj-m	:= dnw.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINC=$(PWD)/../include modules

endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.order *.symvers
depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif



