#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([lysdr], [0.3], [gordon@gjcp.net])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/filter.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Check for required libraries
PKG_CHECK_MODULES(GTK3, gtk+-3.0 >= 3.10)
AC_SEARCH_LIBS(fftw_malloc, fftw3, [],
	AC_MSG_ERROR([unable to find fftw3 library])
)
AC_SEARCH_LIBS(jack_connect, jack, [],
	AC_MSG_ERROR([unable to find jack library])
)
AC_SEARCH_LIBS(sin, m)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove])

dnl Combine the various libs and flags
CFLAGS="$CFLAGS $GTK3_CFLAGS"
LIBS="$LIBS $GTK3_LIBS"

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
