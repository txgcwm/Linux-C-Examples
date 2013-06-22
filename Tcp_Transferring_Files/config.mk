
LDLIBS += -lpthread
LDFLAGS += -g -Wall -O2 

CONFIG_LOOP_BUF = y
ifeq ($(CONFIG_LOOP_BUF),y)
	CFLAGS	+= -D LOOP_BUF
else
	CONFIG_LIST_BUF = y
	ifeq ($(CONFIG_LIST_BUF),y)
		CFLAGS += -D LIST_BUF
	endif
endif