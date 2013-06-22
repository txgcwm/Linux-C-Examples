#

EXEC = dnw

SRCS = main.c 

CFLAGS += -I ../libusb-0.1.12
LDLIBS += -L ../libusb-0.1.12/.libs -lusb
#CFLAGS += -I /usr/local/include
#LDLIBS += -L /usr/local/lib -lusb

$(EXEC): $(SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDLIBS)

clean:
	-rm -f $(EXEC)
