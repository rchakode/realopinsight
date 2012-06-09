#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([ngrt4nd], [2.0], [Rodrigue Chakode <rodrigue.chakode@ngrt4n.com>], [ngrt4nd], [http://ngrt4n.com])
AM_INIT_AUTOMAKE([ngrt4nd], [2.0])

AC_PREREQ([2.68])
AC_CONFIG_HEADERS([config.h])
#AC_CONFIG_SRCDIR([src/core/utils.cpp])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcrypt':
AC_CHECK_LIB(crypt, crypt, [], [
	      echo "libcrypt is required" 
	      exit 1])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB(pthread, pthread_create, [], [
	                    echo "phtread is required" 
			    exit 1])
# FIXME: Replace `main' with a function in `-lzmq':
AC_CHECK_LIB(zmq, zmq_init, [], [
	                    echo "zmq is required" 
			    exit 1])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getpass memset mkdir])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
