
noinst_PROGRAMS = testExpr

DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall
LDADD = $(DDD_BUILDDIR)/libDDD_d.a
# AM_LDFLAGS = -ltbb

# AM_CPPFLAGS = -D INST_STL -I $(DDD_SRCDIR)
# LDADD = $(DDD_BUILDDIR)/libDDD_i.a

testExpr_SOURCES = IntExpression.hpp IntExpression.cpp BoolExpression.hpp BoolExpression.cpp main.cpp

