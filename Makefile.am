ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
		-include $(top_builddir)/src/config.h \
		-I$(top_srcdir)/include \
		-I$(top_builddir)/include

EXTRA_DIST = libmam.pc README.md LICENSE

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmam.pc

###############################################################################
# Includes
###############################################################################

include_HEADERS = \
		include/mam.h \
		include/mam/mirrored-allocators.h \
		include/mam/copy-engine.h

noinst_HEADERS = \
		src/mam-internal.h \
		src/mirrored-allocators-internal.h \
		src/copy-engine-internal.h \
		src/utarray.h

###############################################################################
# Includes
###############################################################################

lib_LTLIBRARIES = libmam.la

libmam_la_SOURCES = \
		src/mirrored-allocators.c \
		src/copy-engine.c

libmam_la_LIBADD = $(LTLIBOBJS)

###############################################################################
# TESTS
###############################################################################

AM_COLOR_TESTS = yes
LDADD = libmam.la

# valgrind support
@VALGRIND_CHECK_RULES@

mam_test_SOURCES = \
		test/mam-test.c

copy_engine_test_SOURCES = \
		test/copy-engine-test.c

GENERAL_TESTS = \
		mam-test \
		copy-engine-test

# unit tests
UNIT_TESTS = \
	     $(GENERAL_TESTS)

check_PROGRAMS = $(UNIT_TESTS)
TESTS = $(UNIT_TESTS)
