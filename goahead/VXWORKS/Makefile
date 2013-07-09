#
# Makefile for the GoAhead web server reference source base
#  for the VxWorks 486 OS
#
# Copyright (c) Go Ahead, 1995-2010
#
#

all:	compile 

CC				= cc386
AR				= ar386
LD				= ld386

ARCH			= libwebs.a
NAME			= webs

# User Management switch
UMSW			= -DUSER_MANAGEMENT_SUPPORT

# Digest Access switch
DASW			= -DDIGEST_ACCESS_SUPPORT

#
#   SSL switches
#   Comment the following 3 lines out to disable SSL
#
SSLINC = ../../matrixssl
SSLLIB = ../../matrixssl/src/libmatrixsslstatic.a
SSLSW   = -DWEBS_SSL_SUPPORT -I$(SSLINC)

WEBCOMP			= webcomp
DEPEND_FILES	= ../asp.o ../balloc.o ../mime64.o ../cgi.o ../default.o  \
				  ../ejlex.o ../ejparse.o ../form.o \
				  ../h.o ../handler.o ../mime.o ../misc.o ../page.o  \
				  ../ringq.o ../rom.o \
				  ../sock.o ../sockGen.o \
				  ../websSSL.o ../matrixSSLSocket.o \
				  ../security.o ../sym.o ../uemf.o ../url.o ../value.o \
				  ../md5c.o ../um.o ../umui.o ../websda.o ../emfdb.o \
				  ../webrom.o ../webs.o ../websuemf.o \
				  main.o
CFLAGS			= -DWEBS -DOS="VXWORKS" -DVXWORKS $(UMSW) $(DASW) -Wall $(SSLSW)
BSP_CFLAGS		= -DCPU=I80486 -fno-builtin -fno-defer-pop -nostdinc -m486
IFLAGS			= -I/tornado2/target/h -I../
OTHERS			= -DB_STATS -DB_FILL -DDEBUG
DEBUG			= -O
#DEBUG			= -g -DDEV -DASSERT
LDFLAGS			= $(SSLLIB)

compile: $(NAME)

#
#	Build archive of objects
#
$(ARCH): $(DEPEND_FILES)
	$(AR) $(ARFLAGS) $(ARCH) $?

#
#	Primary link
#
$(NAME): Makefile $(ARCH)
	$(LD) -o $(NAME).o -r main.o $(ARCH) $(LDFLAGS) 

$(WEBCOMP): Makefile $(ARCH)
	$(LD) -o $(WEBCOMP).o -r webcomp.o $(ARCH) $(LDFLAGS) 

clean:
	rm -f $(NAME) $(ARCH) $(DEPEND_FILES)

#
#	Dependencies
#
../asp.o:  ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../balloc.o: ../balloc.c ../uemf.h

../mime64.o:  ../mime64.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../cgi.o:  ../webs.h ../wsIntrn.h ../uemf.h

../default.o:  ../default.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../emfdb.o:  ../emfdb.h ../wsIntrn.h ../uemf.h

../ejlex.o:  ../ejlex.c ../ej.h ../ejIntrn.h ../uemf.h

../ejparse.o:  ../ejparse.c ../ej.h ../ejIntrn.h ../uemf.h

../form.o:  ../form.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../h.o:  ../h.c ../uemf.h

../handler.o:  ../handler.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../md5c.o:  ../md5.h ../wsIntrn.h ../uemf.h

../mime.o:  ../mime.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../misc.o:  ../misc.c ../uemf.h

../page.o:  ../page.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../ringq.o:  ../ringq.c ../uemf.h

../rom.o:  ../rom.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../security.o:  ../security.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../sock.o:  ../sock.c ../uemf.h

../sockGen.o:  ../sockGen.c ../uemf.h

../sym.o:  ../sym.c ../uemf.h

../uemf.o:  ../uemf.c ../uemf.h

../um.o:  ../webs.h ../wsIntrn.h ../um.h ../uemf.h

../umui.o:  ../webs.h ../wsIntrn.h ../um.h ../uemf.h

../url.o:  ../url.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../value.o:  ../value.c ../uemf.h

../webrom.o:  ../webrom.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../webs.o:  ../webs.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../websda.o:  ../webs.h ../wsIntrn.h ../websda.h ../uemf.h

../websuemf.o:  ../websuemf.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h

../websSSL.o:  ../websSSL.c ../websSSL.h ../wsIntrn.h ../uemf.h

../sock.o:  ../sock.c ../uemf.h

../sockGen.o:  ../sockGen.c ../uemf.h

main.o:  main.c ../wsIntrn.h ../webs.h ../ej.h ../ejIntrn.h ../uemf.h

#
#	Transition rules (add -o to put object in right directory)
#
.SUFFIXES: .c .o

.c.o:
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS) $(BSP_CFLAGS) $(IFLAGS) $<
