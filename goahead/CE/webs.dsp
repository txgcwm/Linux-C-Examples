# Microsoft Developer Studio Project File - Name="webs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE x86em) Application" 0x7f01

CFG=webs - Win32 (WCE x86em) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "webs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "webs.mak" CFG="webs - Win32 (WCE x86em) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "webs - Win32 (WCE x86em) Release" (based on "Win32 (WCE x86em) Application")
!MESSAGE "webs - Win32 (WCE x86em) Debug" (based on "Win32 (WCE x86em) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "H/PC Ver. 2.00"
# PROP WCE_FormatVersion "6.0"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "webs - Win32 (WCE x86em) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x86emRel"
# PROP BASE Intermediate_Dir "x86emRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x86emRel"
# PROP Intermediate_Dir "x86emRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
EMPFILE=empfile.exe
# ADD BASE EMPFILE COPY
# ADD EMPFILE COPY
# ADD BASE CPP /nologo /ML /W3 /O2 /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "NDEBUG" /D "x86" /D "i486" /D "_x86_" /YX /c
# ADD CPP /nologo /ML /W3 /O2 /D "NDEBUG" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "x86" /D "i486" /D "_x86_" /D "CE" /D "UEMF" /D "WEBS" /YX /c
# ADD BASE RSC /l 0x409 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "NDEBUG"
# ADD RSC /l 0x409 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "NDEBUG"
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /subsystem:windows /machine:I386 /windowsce:emulation
# ADD LINK32 winsock.lib commctrl.lib coredll.lib corelibc.lib /nologo /subsystem:windows /incremental:yes /debug /machine:I386 /nodefaultlib:"$(CENoDefaultLib)" /nodefaultlib /windowsce:emulation

!ELSEIF  "$(CFG)" == "webs - Win32 (WCE x86em) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x86emDbg"
# PROP BASE Intermediate_Dir "x86emDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x86emDbg"
# PROP Intermediate_Dir "x86emDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
EMPFILE=empfile.exe
# ADD BASE EMPFILE COPY
# ADD EMPFILE COPY
# ADD BASE CPP /nologo /MLd /W3 /Gm /Zi /Od /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /YX /c
# ADD CPP /nologo /MLd /W3 /Gm /ZI /Od /D "_DEBUG" /D "CE" /D "UEMF" /D "WEBS" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "x86" /D "i486" /D "_x86_" /YX /c
# ADD BASE RSC /l 0x409 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
# ADD RSC /l 0x409 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /subsystem:windows /debug /machine:I386 /windowsce:emulation
# ADD LINK32 winsock.lib commctrl.lib coredll.lib corelibc.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"$(CENoDefaultLib)" /nodefaultlib /windowsce:emulation

!ENDIF 

# Begin Target

# Name "webs - Win32 (WCE x86em) Release"
# Name "webs - Win32 (WCE x86em) Debug"
# Begin Group "Source Files"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\asp.c
DEP_CPP_ASP_C=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_ASP_C=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\balloc.c
DEP_CPP_BALLO=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_BALLO=\
	"..\arpa\inet.h"\
	"..\basic\basicInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\base64.c
DEP_CPP_BASE6=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_BASE6=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\cgi.c
DEP_CPP_CGI_C=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_CGI_C=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\default.c
DEP_CPP_DEFAU=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_DEFAU=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\ejlex.c
DEP_CPP_EJLEX=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_EJLEX=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\ejparse.c
DEP_CPP_EJPAR=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_EJPAR=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\emfdb.c
DEP_CPP_EMFDB=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\emfdb.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_EMFDB=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\form.c
DEP_CPP_FORM_=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_FORM_=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\h.c
DEP_CPP_H_C12=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_H_C12=\
	"..\arpa\inet.h"\
	"..\basic\basicInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\handler.c
DEP_CPP_HANDL=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_HANDL=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=.\main.c
DEP_CPP_MAIN_=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\um.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_MAIN_=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\md5c.c
DEP_CPP_MD5C_=\
	"..\md5.h"\
	
# End Source File
# Begin Source File

SOURCE=..\mime.c
DEP_CPP_MIME_=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_MIME_=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\misc.c
DEP_CPP_MISC_=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_MISC_=\
	"..\arpa\inet.h"\
	"..\basic\basicInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\page.c
DEP_CPP_PAGE_=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_PAGE_=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\ringq.c
DEP_CPP_RINGQ=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_RINGQ=\
	"..\arpa\inet.h"\
	"..\basic\basicInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\rom.c
DEP_CPP_ROM_C=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_ROM_C=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\security.c
DEP_CPP_SECUR=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\um.h"\
	"..\webs.h"\
	"..\websda.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_SECUR=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\sock.c
DEP_CPP_SOCK_=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_SOCK_=\
	"..\arpa\inet.h"\
	"..\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\sockGen.c
DEP_CPP_SOCKG=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_SOCKG=\
	"..\arpa\inet.h"\
	"..\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\sym.c
DEP_CPP_SYM_C=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_SYM_C=\
	"..\arpa\inet.h"\
	"..\basic\basicInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\uemf.c
DEP_CPP_UEMF_=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_UEMF_=\
	"..\arpa\inet.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\um.c
DEP_CPP_UM_C2e=\
	"..\ej.h"\
	"..\emfdb.h"\
	"..\uemf.h"\
	"..\um.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	".\wincompat.h"\
	
NODEP_CPP_UM_C2e=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\emf\emf.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\umui.c
DEP_CPP_UMUI_=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\um.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_UMUI_=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\url.c
DEP_CPP_URL_C=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_URL_C=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\value.c
DEP_CPP_VALUE=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_VALUE=\
	"..\arpa\inet.h"\
	"..\basic\basicInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\webrom.c
DEP_CPP_WEBRO=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_WEBRO=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\webs.c
DEP_CPP_WEBS_=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websda.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_WEBS_=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\websda.c
DEP_CPP_WEBSD=\
	"..\ej.h"\
	"..\md5.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websda.h"\
	"..\websSSL.h"\
	".\wincompat.h"\
	
NODEP_CPP_WEBSD=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\emf\emf.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=..\websuemf.c
DEP_CPP_WEBSU=\
	"..\ej.h"\
	"..\ejIntrn.h"\
	"..\uemf.h"\
	"..\webs.h"\
	"..\websSSL.h"\
	"..\wsIntrn.h"\
	".\wincompat.h"\
	
NODEP_CPP_WEBSU=\
	"..\arpa\inet.h"\
	"..\basic\basic.h"\
	"..\basic\basicInternal.h"\
	"..\ej\ejIntrn.h"\
	"..\emf\emf.h"\
	"..\emf\emfInternal.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	
# End Source File
# Begin Source File

SOURCE=.\wincompat.c
DEP_CPP_WINCO=\
	"..\uemf.h"\
	".\wincompat.h"\
	
NODEP_CPP_WINCO=\
	"..\arpa\inet.h"\
	"..\lf\inmem.h"\
	"..\messages.h"\
	"..\netdb.h"\
	"..\netinet\in.h"\
	"..\sys\select.h"\
	"..\sys\socket.h"\
	".\basic\basicInternal.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\ej.h
# End Source File
# Begin Source File

SOURCE=..\ejIntrn.h
# End Source File
# Begin Source File

SOURCE=..\emfdb.h
# End Source File
# Begin Source File

SOURCE=..\md5.h
# End Source File
# Begin Source File

SOURCE=..\uemf.h
# End Source File
# Begin Source File

SOURCE=..\um.h
# End Source File
# Begin Source File

SOURCE=..\webs.h
# End Source File
# Begin Source File

SOURCE=..\websda.h
# End Source File
# Begin Source File

SOURCE=..\websSSL.h
# End Source File
# Begin Source File

SOURCE=.\wincompat.h
# End Source File
# Begin Source File

SOURCE=..\wsIntrn.h
# End Source File
# End Group
# End Target
# End Project
