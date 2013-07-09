EXTRA_DIST = libinifile++.pc libinifile++.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libinifile++.pc

lib_LTLIBRARIES = libinifile++.la
libinifile___la_SOURCES = \
	../inifile.c ../lexer.c ../parser.c ../token.c \
	inifile++.hpp inifile++.cpp 
libinifile___la_LDFLAGS = -version-info 4:0:0 

INCLUDES = -I$(top_srcdir)/src/cxx -I$(top_srcdir)/src

library_includedir=$(includedir)
library_include_HEADERS = inifile++.hpp
