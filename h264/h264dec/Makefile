ARM_TOOLCHAIN = /home/edsiper/ToolChain/Mozart_Toolchain/
ARM_ROOT      = $(ARM_TOOLCHAIN)/arm-eabi-uclibc
CFLAGS  = -O0 -g3 -Wall -Wno-format-security
OBJ     = streamer.o network.o base64.o utils.o rtp.o rtcp.o rtsp.o h264dec.o
SOURCES = streamer.c network.c base64.c utils.c rtp.c rtcp.c rtsp.c h264dec.c
LIBS    = -lpthread

all: h264dec

h264dec: $(OBJ)
	gcc $(CFLAGS) -o $@ $(OBJ) $(LIBS)

arm: $(OBJ)
	arm-linux-gcc --sysroot=$(ARM_ROOT) $(SOURCES) -o h264dec $(LIBS)

clean:
	rm -rf *~ *.o rtsp

