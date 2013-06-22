
EXEC = camera

CFLAGS = -g 
CFLAGS += -I $(TOP_DIR)/include -I $(TOP_DIR)/x264
LDFLAGS += -L $(TOP_DIR)/x264
LDLIBS += -lpthread -lx264 -lm

SRCS = main.c \
		capture.c \
		h264encoder.c \

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(EXEC) *.o *.h264
