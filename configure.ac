#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([console-secrets], [csm-0.2.0], [https://github.com/gridge/console-secrets/issues])
AC_CONFIG_SRCDIR([src/csm.cc])
AC_CONFIG_HEADERS([config.h])

# Command-line arguments
AC_ARG_WITH(debug, [  --with-debug            enable debug])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_INSTALL
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN"
then
	AC_MSG_WARN([Doxygen not found. You will not be able to build source documentation])
fi

# Checks for libraries.
misslib=0
AC_CHECK_LIB([menu], [new_menu], [], [misslib=1])
AC_CHECK_LIB([ncurses], [initscr], [], [misslib=1])
AC_CHECK_LIB([form], [new_form], [], [misslib=1])

# Checks for header files.
AC_CHECK_HEADERS([locale.h stdlib.h string.h unistd.h], [], [misslib=1])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([setlocale])

# If any check failed up to here, abort
if test $misslib == 1
then
	AC_MSG_ERROR([Missing requirement (see above)])
fi

# Check if we want debug information enabled
if test "$with-debug" == "yes"
then
	AC_SUBST([enabledebug], [-g])
else
	AC_SUBST([enabledebug], [])
fi

# Write output Makefile
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

########################### SCRATCH

## This is still under development. 
## For now it's more a scratch area.
## Things will come together soon
#############################

## GpgMe library
#To check if we have gpg enables use the following macro (which will setup the GPGME_* variables too
#Macro: AM_PATH_GPGME ([minimum-version], [action-if-found], [action-if-not-found])
#Use macro AC_SYS_LARGEFILE to enable large-file-support for the application
#AM_CPPFLAGS = $(GPGME_CFLAGS)
#LDADD = $(GPGME_LIBS)
