EXTRA_DIST = libinifile.pc libinifile.pc.in

noinst_HEADER = common.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libinifile.pc

lib_LTLIBRARIES = libinifile.la
libinifile_la_SOURCES = inifile.h inifile.c parser.c lexer.c token.c common.h
libinifile_la_LDFLAGS = -version-info 3:0:0

library_includedir=$(includedir)
library_include_HEADERS = inifile.h
