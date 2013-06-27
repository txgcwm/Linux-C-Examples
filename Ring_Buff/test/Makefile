SERVER = server
CLIENT = client

SERVER_SRCS = server.c \
			  ../ringbuff.c
CLIENT_SRCS = client.c

CFLAGS += -I ../
LDLIBS += -lpthread
LDFLAGS += -g -Wall -O2 

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRCS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(CLIENT): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
clean:
	rm -f $(SERVER) $(CLIENT) test.h264

