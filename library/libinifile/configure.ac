# -*- sh -*- -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libinifile, 0.5.5, [Anders Lövgren <lespaul@algonet.se>])
AM_INIT_AUTOMAKE(libinifile, 0.5.5)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
OPTFCC="$CFLAGS"
OPTFPP="$CXXFLAGS"
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET
CFLAGS=""
CXXFLAGS=""
LIBINIFILE_CHECK_COMPILERS
CFLAGS="$CFLAGS $OPTFCC"
CXXFLAGS="$CXXFLAGS $OPTFPP"

# Use C++ compiler for library and header check.
AC_LANG(C++)
  
# Check compiler carecteristics and options.
AC_CXX_NAMESPACES
AC_CXX_EXCEPTIONS

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC

# Check STL headers:
AC_CHECK_HEADER(string,   AC_DEFINE(HAVE_STL_STRING,  1,[Define to 1 if you have the C++ STL <string> header file.]))
AC_CHECK_HEADER(fstream,  AC_DEFINE(HAVE_STL_FSTREAM, 1,[Define to 1 if you have the C++ STL <fstream> header file.]))
AC_CHECK_HEADER(iostream, AC_DEFINE(HAVE_STL_IOSTREAM,1,[Define to 1 if you have the C++ STL <iostream> header file.]))
AC_CHECK_HEADER(sstream,  AC_DEFINE(HAVE_STL_SSTREAM, 1,[Define to 1 if you have the C++ STL <sstream> header file.]))

# Conditional tested headers:
if test "${ac_cv_header_iostream}" != "yes"; then
  AC_CHECK_HEADER([iostream.h])
fi
if test "${ac_cv_header_fstream}" != "yes"; then
  AC_CHECK_HEADER([fstream.h])
fi
if test "${ac_cv_header_sstream}" != "yes"; then
  AC_CHECK_HEADER([strstream.h])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getline memmove memset strchr])

# Add replacement for getline and getdelim (GNU extensions):
if test "x$ac_cv_func_getline" != "xyes"; then
  AC_LIBOBJ(getdelim)
fi

# Check if C++ library should be built.
LIBINIFILE_ENABLE_CXXLIB
AM_CONDITIONAL(BUILD_INIFILE_CXXLIB, test x${libinifile_build_cxxlib} = xtrue)

# Check if test programs should be built
LIBINIFILE_ENABLE_TESTS
build_inifile_tests_cxx=false
if test x${libinifile_build_test} = xtrue -a x${libinifile_build_cxxlib} = xtrue; then
  build_inifile_tests_cxx=true
fi
AM_CONDITIONAL(BUILD_INIFILE_TESTS, test x${libinifile_build_test} = xtrue)
AM_CONDITIONAL(BUILD_INIFILE_TESTS_CXX, test x${build_inifile_tests_cxx} = xtrue)

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 src/cxx/Makefile
		 src/test/Makefile
		 docs/Makefile
		 src/libinifile.pc
		 src/cxx/libinifile++.pc])
AC_OUTPUT
