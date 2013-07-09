#
# Makefile for the GoAhead web server reference source base
#  for the NetWare OS
#
# Copyright (c) GoAhead Software, Inc. 1995-2010
#
#

pvmaj       = 2           # major version number
pvmin       = 5           # minor version number
pvrev       = 0           # revision number e.g. 0,1,2, ...

description = GoAhead web server 

.EXTENSIONS:
.EXTENSIONS: .exe .rex .lib .obj .asm .c 

all:	compile 

NAME	= webs

copyright = Copyright (c) GoAhead Software, Inc. 1995-2010

# User Management switch
UMSW	= -DUSER_MANAGEMENT_SUPPORT

# Digest Access switch
DASW	= -DDIGEST_ACCESS_SUPPORT

#	SSL switches
#   Comment the following 3 lines out to disable SSL
#
SSLINC = ../../matrixssl
SSLLIB = ../../matrixssl/src/libmatrixsslstatic.a
SSLSW   = -DWEBS_SSL_SUPPORT -I$(SSLINC)

# IFMODSW = -DWEBS_IF_MODIFIED_SUPPORT
# IFMODLIB = /usr/lib/libm.a

# Dependencies
DEPEND_FILES	=                     main.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\asp.obj
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\balloc.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\mime64.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\cgi.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\default.obj  
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\ejlex.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\ejparse.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\form.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\h.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\handler.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\mime.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\misc.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\page.obj  
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\ringq.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\rom.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\sock.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\sockGen.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\websSSL.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\matrixSSLSocket.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\security.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\sym.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\uemf.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\url.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\value.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\md5c.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\um.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\umui.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\websda.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\emfdb.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\webrom.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\webs.obj 
DEPEND_FILES	= $+$(DEPEND_FILES)$- ..\websuemf.obj 
              
!ifndef %qmkver
!   define version d                # use 'd' or 'p' here
!else
!   define version $(%qmkver)
!endif
!ifeq version d
!   define lversion DEBUG
!   define debug /dDEBUG 
!else
!   define lversion PRODUCTION
!   define debug
!endif

startup  = $(nlm386imp)\prelude.obj
nlm_TYPE = Type 0 Description "$(description)"
nlm_NAME = Output $^&
!ifeq version d
!   define nlm_SCREEN ScreenName "webs"
!else
!   define nlm_SCREEN ScreenName "default"
!endif
nlm_THREAD = ThreadName "$^& (main)"
nlm_STACK = Stack 8192
nlm_NLMVER = Version $(pvmaj),$(pvmin),$(pvrev)
nlm_COPYRIGHT = Copyright "$(copyright)"
!ifeq version d
!   define linkop $+$(linkop)$- Map
!   define ldebug debug
!endif
            
import = $(clibimp) $(nlm386imp)\socklib.imp $(nlm386imp)\streams.imp

module = CLib

build_msg = Building a $(lversion) version of $(description) ($(program))

#pgm_ver = /dvMAJ="$(pvmaj)" /dvMIN="$(pvmin)" /dvREV="$(pvrev)"
pgm_ver = 

d_wcc386opt = /dNW /ez /ms /w4 /e99 /zp1 /3s /ot /d1 /zq $(debug)
p_wcc386opt = /dNW /ez /ms /w4 /s /zp1 /3s /oaxt /zq 
x_wcc386opt = $($(version)_wcc386opt) $(pgm_ver)

compiler_cmd = $(wcc386) $(x_wcc386opt) $[*.c


.BEFORE
    echo $(build_msg)
    set inc386=$(inc_386);.\
    set lib386=$(lib_386)
    set wcg386=$(code_386)

.DEFAULT:
        echo Now we create $@ from $?
        $(compiler_cmd) /fo=$@

.c.obj:
        echo Now we create $@ from $?
        $(compiler_cmd) /fo=$@

        
compile: $(NAME)

all : $(programs) .symbolic
        @%null

$(NAME).NLM : $(DEPEND_FILES) $(startup) $(NAME).DEF
        $(nlmlinkr) $(NAME)


$(NAME).DEF : MAKEFILE
  if exist $^&.DEF del $^&.DEF
  %append $^&.DEF $(nlm_TYPE)
  %append $^&.DEF $(nlm_NAME)
  %append $^&.DEF $(nlm_SCREEN)
  %append $^&.DEF $(nlm_THREAD)
  %append $^&.DEF $(nlm_STACK)
  %append $^&.DEF $(nlm_CHECK)
  %append $^&.DEF $(nlm_NLMVER)
  %append $^&.DEF $(nlm_MESSAGE)
  %append $^&.DEF $(nlm_HELP)
!ifdef copyright
  %append $^&.DEF $(nlm_COPYRIGHT)
!endif
!ifdef ldebug
  %append $^&.DEF $(ldebug)
!endif
  for %i in ($(linkop))  do %append $^&.DEF %i
  for %i in ($(DEPEND_FILES))  do %append $^&.DEF Input %i
  for %i in ($(startup)) do %append $^&.DEF Input %i
  for %i in ($(import))  do %append $^&.DEF Import @%i
  for %i in ($(export))  do %append $^&.DEF Export @%i
  for %i in ($(module))  do %append $^&.DEF Module %i


#
#	Build archive of objects
#
#$(ARCH): $(DEPEND_FILES)
#	$(AR) $(ARFLAGS) $(ARCH) $?

#
#	Primary link
#
$(NAME): Makefile $(ARCH)
	$(CC) -o $(NAME) $(DEBUG) \
		$(CFLAGS) $(IFLAGS) \
		main.obj $(ARCH) $(LDFLAGS) 

clean:
	rm -f $(NAME) $(ARCH)

#
#	Dependencies
#
..\asp.obj:  ..\asp.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\balloc.obj: ..\balloc.c ..\uemf.h

..\mime64.obj:  ..\mime64.c ..\webs.h ..\wsIntrn.h  ..\ej.h ..\ejIntrn.h ..\uemf.h

..\cgi.obj:  ..\cgi.c ..\webs.h ..\wsIntrn.h ..\uemf.h

..\default.obj:  ..\default.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\ejlex.obj:  ..\ejlex.c ..\ej.h ..\ejIntrn.h ..\uemf.h

..\ejparse.obj:  ..\ejparse.c ..\ej.h ..\ejIntrn.h ..\uemf.h

..\emfdb.obj:  ..\emfdb.c ..\emfdb.h ..\wsIntrn.h ..\uemf.h

..\form.obj:  ..\form.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\h.obj:  ..\h.c ..\uemf.h

..\handler.obj:  ..\handler.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\md5c.obj:  ..\md5c.c ..\md5.h ..\wsIntrn.h ..\uemf.h

..\mime.obj:  ..\mime.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\misc.obj:  ..\misc.c ..\uemf.h

..\page.obj:  ..\page.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\ringq.obj:  ..\ringq.c ..\uemf.h

..\rom.obj:  ..\rom.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\security.obj:  ..\security.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\sock.obj:  ..\sock.c ..\uemf.h

..\sockGen.obj:  ..\sockGen.c ..\uemf.h

..\sym.obj:  ..\sym.c ..\uemf.h

..\uemf.obj:  ..\uemf.c ..\uemf.h

..\um.obj:  ..\um.c ..\webs.h ..\wsIntrn.h ..\um.h ..\uemf.h

..\umui.obj:  ..\umui.c ..\webs.h ..\wsIntrn.h ..\um.h ..\uemf.h

..\url.obj:  ..\url.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\value.obj:  ..\value.c ..\uemf.h

..\webrom.obj:  ..\webrom.c ..\webs.h ..\wsIntrn.h ..\uemf.h

..\webs.obj:  ..\webs.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\websda.obj:  ..\websda.c ..\webs.h ..\wsIntrn.h ..\websda.h ..\uemf.h

..\websuemf.obj:  ..\websuemf.c ..\webs.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

..\websSSL.obj:  ..\websSSL.c ..\websSSL.h ..\wsIntrn.h ..\ej.h ..\ejIntrn.h ..\uemf.h

main.obj:  main.c ..\wsIntrn.h ..\webs.h ..\ej.h ..\ejIntrn.h ..\uemf.h

