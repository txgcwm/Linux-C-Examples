dnl @synopsis AC_CXX_NAMESPACES
dnl
dnl If the compiler can prevent names clashes using namespaces, define
dnl HAVE_NAMESPACES.
dnl
dnl @category Cxx
dnl @author Todd Veldhuizen
dnl @author Luc Maisonobe <luc@spaceroots.org>
dnl @version 2004-02-04
dnl @license AllPermissive
AC_DEFUN([AC_CXX_NAMESPACES],
[AC_CACHE_CHECK(whether the compiler implements namespaces,
ac_cv_cxx_namespaces,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([namespace Outer { namespace Inner { int i = 0; }}],
                [using namespace Outer::Inner; return i;],
 ac_cv_cxx_namespaces=yes, ac_cv_cxx_namespaces=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_namespaces" = yes; then
  AC_DEFINE(HAVE_NAMESPACES,,[define if the compiler implements namespaces])
fi
])

dnl @synopsis AC_CXX_EXCEPTIONS
dnl
dnl If the C++ compiler supports exceptions handling (try, throw and
dnl catch), define HAVE_EXCEPTIONS.
dnl
dnl @category Cxx
dnl @author Todd Veldhuizen
dnl @author Luc Maisonobe <luc@spaceroots.org>
dnl @version 2004-02-04
dnl @license AllPermissive

AC_DEFUN([AC_CXX_EXCEPTIONS],
[AC_CACHE_CHECK(whether the compiler supports exceptions,
ac_cv_cxx_exceptions,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE(,[try { throw  1; } catch (int i) { return i; }],
 ac_cv_cxx_exceptions=yes, ac_cv_cxx_exceptions=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_exceptions" = yes; then
  AC_DEFINE(HAVE_EXCEPTIONS,,[define if the compiler supports exceptions])
fi
])

dnl Borroved from KDE

AC_DEFUN([LIBINIFILE_CHECK_COMPILER_FLAG],
[
AC_MSG_CHECKING([whether $CXX supports -$1])
libinifile_cache=`echo $1 | sed 'y% .=/+-,%____p__%'`
AC_CACHE_VAL(libinifile_cv_prog_cxx_$libinifile_cache,
[
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -$1"
  AC_TRY_LINK([],[ return 0; ], [eval "libinifile_cv_prog_cxx_$libinifile_cache=yes"], [])
  CXXFLAGS="$save_CXXFLAGS"
  AC_LANG_RESTORE
])
if eval "test \"`echo '$libinifile_cv_prog_cxx_'$libinifile_cache`\" = yes"; then
 AC_MSG_RESULT(yes)
	:
 $2
else
 AC_MSG_RESULT(no)
	:
 $3
fi
])

AC_DEFUN([LIBINIFILE_CHECK_COMPILERS],
[
  AC_ARG_ENABLE(debug,
                AC_HELP_STRING([--enable-debug=ARG],[enables debug symbols (yes|no|full) [[default=no]]]),
  [
    case $enableval in
      yes)
        libinifile_use_debug_code="yes"
        libinifile_use_debug_define=no
        ;;
      full)
        libinifile_use_debug_code="full"
        libinifile_use_debug_define=no
        ;;
      *)
        libinifile_use_debug_code="no"
        libinifile_use_debug_define=yes
        ;;
    esac
  ],
    [libinifile_use_debug_code="no"
     libinifile_use_debug_define=no
  ])
  
  AC_ARG_ENABLE(warnings,AC_HELP_STRING([--disable-warnings],[disables compilation with -Wall and similar]),
   [
    if test $enableval = "no"; then
         libinifile_use_warnings="no"
       else
         libinifile_use_warnings="yes"
    fi
   ], [libinifile_use_warnings="yes"])

  AC_ARG_ENABLE(profile,AC_HELP_STRING([--enable-profile],[creates profiling info [[default=no]]]),
    [libinifile_use_profiling=$enableval],
    [libinifile_use_profiling="no"]
  )

  dnl this prevents stupid AC_PROG_CC to add "-g" to the default CFLAGS
  
  FLAGSCXX=" $CXXFLAGS"
  CXXFLAGS=" "
  AC_PROG_CXX

  FLAGSC=" $CFLAGS"
  CFLAGS=" "
  AC_PROG_CC

  AC_PROG_CPP
  AC_PROG_CXXCPP

  if test "$GCC" = "yes"; then
    if test "$libinifile_use_debug_code" != "no"; then
      if test $libinifile_use_debug_code = "full"; then
        CFLAGS="-g3 -fno-inline -DDEBUG $CFLAGS"
      else
        CFLAGS="-g -O -fno-reorder-blocks -fno-schedule-insns -fno-inline $CFLAGS"
      fi
    else
      CFLAGS="-O2 $CFLAGS"
    fi
  fi

  if test "$GXX" = "yes" || test "$CXX" = "KCC"; then
    if test "$libinifile_use_debug_code" != "no"; then
      if test "$CXX" = "KCC"; then
        CXXFLAGS="+K0 -Wall -pedantic -W -Wpointer-arith -Wwrite-strings $CXXFLAGS"
      else
        if test "$libinifile_use_debug_code" = "full"; then
          CXXFLAGS="-g3 -fno-inline -DDEBUG $CXXFLAGS"
        else
          CXXFLAGS="-g -O -fno-reorder-blocks -fno-schedule-insns -fno-inline $CXXFLAGS"
        fi
      fi
      LIBINIFILE_CHECK_COMPILER_FLAG(fno-builtin,[CXXFLAGS="-fno-builtin $CXXFLAGS"])

      dnl convenience compiler flags
      LIBINIFILE_CHECK_COMPILER_FLAG(Woverloaded-virtual, [WOVERLOADED_VIRTUAL="-Woverloaded-virtual"], [WOVERLOADED_VRITUAL=""])
      AC_SUBST(WOVERLOADED_VIRTUAL)
    else
      if test "$CXX" = "KCC"; then
        CXXFLAGS="+K3 $CXXFLAGS"
      else
        CXXFLAGS="-O2 $CXXFLAGS"
      fi
    fi
  fi

  if test "$libinifile_use_debug_define" = "yes"; then
    CXXFLAGS="-DNDEBUG -DNO_DEBUG $CXXFLAGS"
    CFLAGS="-DNDEBUG $CFLAGS"
  fi

  if test "$libinifile_use_profiling" = "yes"; then
    LIBINIFILE_CHECK_COMPILER_FLAG(pg,
    [
      CXXFLAGS="-pg $CXXFLAGS"
      CFLAGS="-pg $CFLAGS"
     ])
  fi

  if test "$libinifile_use_warnings" = "yes"; then
      if test "$GCC" = "yes"; then
        CXXFLAGS="-Wall -W -Wpointer-arith $CXXFLAGS"
        if test "$libinifile_use_debug_code" != "no"; then
	  CXXFLAGS="-Werror $CXXFLAGS"
	fi
        case $host in
          *-*-linux-gnu)
            CFLAGS="-std=iso9899:1990 -W -Wall -Wchar-subscripts -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -D_XOPEN_SOURCE=600 -D_BSD_SOURCE $CFLAGS"
            CXXFLAGS="-ansi -D_XOPEN_SOURCE=600 -D_BSD_SOURCE -Wcast-align -Wconversion -Wchar-subscripts $CXXFLAGS"
            LIBINIFILE_CHECK_COMPILER_FLAG(Wmissing-format-attribute, [CXXFLAGS="$CXXFLAGS -Wformat-security -Wmissing-format-attribute"])
          ;;
        esac
        LIBINIFILE_CHECK_COMPILER_FLAG(Wundef,[CXXFLAGS="-Wundef $CXXFLAGS"])
        LIBINIFILE_CHECK_COMPILER_FLAG(Wno-long-long,[CXXFLAGS="-Wno-long-long $CXXFLAGS"])
        LIBINIFILE_CHECK_COMPILER_FLAG(Wno-non-virtual-dtor,[CXXFLAGS="$CXXFLAGS -Wno-non-virtual-dtor"])
     fi
  fi

  if test "$GXX" = "yes"; then
    LIBINIFILE_CHECK_COMPILER_FLAG(fno-check-new, [CXXFLAGS="$CXXFLAGS -fno-check-new"])
    LIBINIFILE_CHECK_COMPILER_FLAG(fno-common, [CXXFLAGS="$CXXFLAGS -fno-common"])
    LIBINIFILE_CHECK_COMPILER_FLAG(fexceptions, [CXXFLAGS="$CXXFLAGS -fexceptions"])
    ENABLE_PERMISSIVE_FLAG="-fpermissive"
  fi
  
   case "$host" in
      *-*-irix*)  test "$GXX" = yes && CXXFLAGS="-D_LANGUAGE_C_PLUS_PLUS -D__LANGUAGE_C_PLUS_PLUS $CXXFLAGS" ;;
      *-*-sysv4.2uw*) CXXFLAGS="-D_UNIXWARE $CXXFLAGS";;
      *-*-sysv5uw7*) CXXFLAGS="-D_UNIXWARE7 $CXXFLAGS";;
      *-*-solaris*)
        if test "$GXX" = yes; then
          libstdcpp=`$CXX -print-file-name=libstdc++.so`
          if test ! -f $libstdcpp; then
             AC_MSG_ERROR([You've compiled gcc without --enable-shared. This doesn't work with KDE. Please recompile gcc with --enable-shared to receive a libstdc++.so])
          fi
        fi
        ;;
  esac
 
  
  dnl Merge with flags given on command line.
  CXXFLAGS="$CXXFLAGS $FLAGSCXX"
  CFLAGS="$CFLAGS $FLAGSC"
])

AC_DEFUN([LIBINIFILE_ENABLE_TESTS],	
[
  AC_ARG_ENABLE([test],
  		[  --enable-test           build test apps (yes|no) [[default=no]]],
	 	[case "${enableval}" in
	           yes) 
		     libinifile_build_test=true 
		     ;;
		   no)  
		     libinifile_build_test=false 
		     ;;
		   *) 
		     AC_MSG_ERROR(bad value ${enableval} for --enable-test) 
		     ;;
          	 esac],
	  	[libinifile_build_test=false])
])

AC_DEFUN([LIBINIFILE_ENABLE_CXXLIB],
[
  AC_ARG_ENABLE([cxx],
  		[  --enable-cxx            build C++ version of library [[default=no]]],
		[case "${enableval}" in
 		   yes) 
		     libinifile_build_cxxlib=true 
		     ;;
		   no) 
		     libinifile_build_cxxlib=false 
		     ;;
		   *) 
		     AC_MSG_ERROR(bad value ${enableval} for --enable-cxx) 
		     ;;
 		 esac],
		[libinifile_build_cxxlib=false])
])

