check_PROGRAMS = tst

check_SCRIPTS = uttk.sh

DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall
LDADD = $(DDD_BUILDDIR)/libDDD_d.a
tst_SOURCES = tst.cc dot_representation.cc

TESTS = uttk.sh
