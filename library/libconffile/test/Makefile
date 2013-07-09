BIN=test

CFLAGS  += -I $(CONF_DIR)/src
LDFLAGS += -L $(CONF_DIR)/src
LDLIBS += -lconf
SRCS=test.c

$(BIN): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS) -static

clean:
	rm -f $(BIN)
