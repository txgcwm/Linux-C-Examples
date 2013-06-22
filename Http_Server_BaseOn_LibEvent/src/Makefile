EXEC = http_server

CFLAGS += -I ../include
LDLIBS += -levent
SRCS = http_server.c \
		getoptions.c \
		log.c \
		response.c \

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS) -g -Wall -O2

clean:
	rm -f $(EXEC)