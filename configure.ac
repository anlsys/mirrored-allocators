# init
######

AC_INIT([libmam],[0.1.0],[bvideau@anl.gov])
AC_CONFIG_SRCDIR([include/mam.h])
AC_CONFIG_AUX_DIR([m4])
AC_CONFIG_MACRO_DIR([m4])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects 1.12])

# check for programs
####################
AC_LANG([C])
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_TYPE_SIZE_T
AC_TYPE_INTPTR_T
AM_PROG_AR
LT_INIT

# support for testing with valgrind
###################################

AX_VALGRIND_DFLT([helgrind], [off])
AX_VALGRIND_DFLT([drd], [off])
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

# output config
###################################
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([
	Makefile
	libmam.pc
])
AC_OUTPUT
