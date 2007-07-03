noinst_PROGRAMS = terms



DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall
LDADD = $(DDD_BUILDDIR)/libDDD_d.a

# AM_CPPFLAGS = -D INST_STL -I $(DDD_SRCDIR)
# LDADD = $(DDD_BUILDDIR)/libDDD_i.a

terms_SOURCES = Terms.cpp Nat_PlusZero.hpp Nat_PlusSucc.hpp Nat_Const.hpp Nat_Const.cpp




