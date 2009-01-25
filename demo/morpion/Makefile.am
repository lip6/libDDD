noinst_PROGRAMS = morpionv2 morpionv1


DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall $(TBBINC_FLAGS)
LDADD = $(DDD_BUILDDIR)/libDDD_d.a

morpionv1_SOURCES = morpion.cpp \
	morpion_hom.cpp morpion_hom.hpp \
	Hom_Basic.hh Hom_Basic.cpp

morpionv2_SOURCES = morpionv2.cpp \
	morpion_hom_v2.cpp morpion_hom_v2.hpp \
	morpion_hom.cpp morpion_hom.hpp \
	Hom_Basic.hh Hom_Basic.cpp

