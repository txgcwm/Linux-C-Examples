CFLAGS = -Wall -g 
EXECUTABLE = wav_parse

OBJS = wav_parse.o main.o

default:$(EXECUTABLE)

$(EXECUTABLE):$(OBJS)
	$(CC) -o $(EXECUTABLE) $(CFLAGS) $(OBJS) 

%.o:%.c
	$(CC) -c -o $@ $< $(CFLAGS) 

clean:
	rm -f $(EXECUTABLE) $(OBJS)

