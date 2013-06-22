
EXEC = desktop

SRCS = desktop-entry-util.c  desktop-list.c desktop.c

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: clean

clean:
	$(RM) $(EXEC)
