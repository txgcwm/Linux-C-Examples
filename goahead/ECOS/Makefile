#
# Makefile for the GoAhead web server reference source base
#  for the eCos OS
#
# Copyright (c) GoAhead Software, Inc. 1995-2010
# 

all: compile

#
# These definitions come from your eCos install tree
#
DEBUG	:= -g -Wall -O2

# For Cirrus Logic EDB72xx board
PKG_INSTALL_DIR := /work/net_test/install
COMMAND_PREFIX := arm-elf-
CFLAGS := -mcpu=arm7tdmi -D__EDB7209 -D__EDB7212 $(DEBUG)

# For Motorola PowerPC MBX/860
##PKG_INSTALL_DIR := /work/net_mbx/install
##COMMAND_PREFIX := powerpc-eabi-
##CFLAGS := -mcpu=860 -msoft-float $(DEBUG)

# 
# These should not need to be changed
#
CC := $(COMMAND_PREFIX)gcc
OBJCOPY := $(COMMAND_PREFIX)objcopy
AR := $(COMMAND_PREFIX)ar
LDFLAGS = -nostartfiles -L$(PKG_INSTALL_DIR)/lib -Wl,--gc-sections $(LIBS)
LIBS = -Ttarget.ld -nostdlib
CXXFLAGS      = $(CFLAGS)
EXTRACFLAGS   = -Wall -I$(PKG_INSTALL_DIR)/include -ffunction-sections -fdata-sections
EXTRACXXFLAGS = $(EXTRACFLAGS) -fno-exceptions -fno-rtti -fvtable-gc -finit-priority

# eCos build rules
%.o: %.c
	$(CC) -c -o $*.o $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $<

%.d: %.c
	$(CC) -E $(CFLAGS) $(EXTRACFLAGS) -Wp,-MD,$*.d $< >/dev/null

%.o: %.cxx
	$(CXX) -c -o $*.o $(CXXFLAGS) $(EXTRACXXFLAGS) $<

%.o: %.C
	$(CXX) -c -o $*.o $(CXXFLAGS) $(EXTRACXXFLAGS) $<

%.o: %.cc
	$(CXX) -c -o $*.o $(CXXFLAGS) $(EXTRACXXFLAGS) $<


#------------------- GoAheadWebServer stuff --------------------
ARCH	= libwebs.a
NAME	= webs
OBJ_FILES	= ../asp.o ../balloc.o ../mime64.o ../default.o  \
		  ../ejlex.o ../ejparse.o ../form.o \
		  ../h.o ../handler.o ../mime.o ../misc.o ../page.o  \
		  ../ringq.o ../rom.o \
		  ../sock.o ../sockGen.o \
          ../websSSL.o ../matrixSSLSocket.o \
		  ../security.o ../sym.o ../uemf.o ../url.o ../value.o \
          ../md5c.o ../um.o ../umui.o ../websda.o ../emfdb.o \
		  ../webrom.o ../webs.o ../websuemf.o
DEPEND_FILES = $(patsubst %.o,%.d,$(OBJ_FILES) main.o)
CFLAGS	+= -DWEBS -DWEBS_PAGE_ROM -DOS="eCos" -DECOS -D__ECOS -D__NO_FCNTL=1 -I..

#SSLINC = ../../matrixssl
#SSLLIB = ../../matrixssl/src/libmatrixsslstatic.a
#CFLAGS += -DUSER_MANAGEMENT_SUPPORT -DDIGEST_ACCESS_SUPPORT -DWEBS_SSL_SUPPORT -I$(SSLINC)

.PHONY: depend clean

compile: $(NAME)

depend: $(DEPEND_FILES)
	find .. -name "*.d" | xargs cat >.depend

#
#	Build archive of objects
#
$(ARCH): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $(ARCH) $?

#
#	Primary link
#
$(NAME): Makefile  main.o $(ARCH)
	$(CC) -o $(NAME) $(CFLAGS) $(IFLAGS) \
		main.o $(ARCH) $(LDFLAGS) 

clean:
	rm -f $(NAME) $(ARCH) $(DEPEND_FILES) $(OBJ_FILES) 
	rm -f main.o webrom.c webcomp web_files .depend

# 
# This tool needs to be built using the native C compiler
#
webcomp:
	cc -o webcomp -O2 -DWEBS -DOS="Linux" -DLINUX -I.. ../webcomp.c

#
# Build a set of ROMable pages
#
webrom.c: webcomp
	find ../web -name "*.*" >web_files
	./webcomp ../web web_files >webrom.c

# Dependencies

-include .depend
