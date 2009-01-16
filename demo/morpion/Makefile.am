noinst_PROGRAMS = morpionv1


DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall $(TBBINC_FLAGS)
LDADD = $(DDD_BUILDDIR)/libDDD_d.a

morpionv1_SOURCES = morpion.cpp morpion_hom.hpp morpion_hom.cpp

