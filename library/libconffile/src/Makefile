LIBCONF=libconf.a
LIBCONF_SHARED=libconf.so
LIBCONF_VERS=0.1

CFLAGS = -I ./
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

all: $(OBJS)
	$(AR) -rc $(LIBCONF) $(OBJS)
	$(CC) -shared -fPCI -o $(LIBCONF_SHARED) $(OBJS)

clean:
	rm -f *.[oa] $(LIBCONF)* $(LIBCONF_SHARED)*
