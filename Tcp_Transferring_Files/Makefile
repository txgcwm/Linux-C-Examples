include config.mk

SERVER = server
CLIENT = client

SERVER_SRCS = server.c
CLIENT_SRCS = client.c

ifeq ($(CONFIG_LOOP_BUF),y)
SERVER_SRCS += loop_buf.c
else
SERVER_SRCS += list_buf.c
endif

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRCS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(CLIENT): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
clean:
	rm -f $(SERVER) $(CLIENT) test.*