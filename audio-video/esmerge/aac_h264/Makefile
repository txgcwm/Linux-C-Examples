EXEC = esmerge

SRCS = esmerge.c

CFLAGS += -I ../tstools
CFLAGS += -L ../tstools
LDLIBS += -ltstools -lm -lpthread
LDFLAGS += -g -Wall -O2

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	
clean:
	rm -f $(EXEC) test.*
