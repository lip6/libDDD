noinst_PROGRAMS = morpionv2


DDD_SRCDIR = $(top_srcdir)
DDD_BUILDDIR = $(top_builddir)/ddd

AM_CPPFLAGS =  -I $(DDD_SRCDIR) $(BOOST_CPPFLAGS) -g -Wall $(TBBINC_FLAGS)
AM_LDFLAGS = $(LDFLAGS)
AM_LDFLAGS += $(STATICFLAGS)
LDADD = $(DDD_BUILDDIR)/libDDD_d.la


morpionv2_SOURCES = \
      hom/notew.cpp \
			hom/play.cpp \
			hom/winner.cpp \
			hom/general_v2.cpp \
			hom/const.cpp \
			morpionv2.cpp


noinst_HEADERS = \
      hom/notew.hpp \
			hom/play.hpp \
			hom/winner.hpp \
			hom/general_v2.hpp \
			hom/const.hpp
