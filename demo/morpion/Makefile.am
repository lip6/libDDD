noinst_PROGRAMS =
if HAVE_BOOST
  noinst_PROGRAMS += morpionv2
endif

DDD_SRCDIR = $(top_srcdir)/src ./
DDD_BUILDDIR = $(top_builddir)/src

BASE = hom
AM_CPPFLAGS =  -I $(DDD_SRCDIR) -I $(BOOST_CPPFLAGS) -g -Wall $(TBBINC_FLAGS)
LDADD = $(DDD_BUILDDIR)/libDDD_d.a


morpionv2_SOURCES = 	$(BASE)/notew.cpp \
			$(BASE)/play.cpp $(BASE)/winner.cpp \
			$(BASE)/general_v2.cpp \
			morpionv2.cpp


noinst_HEADERS = 	$(BASE)/notew.hpp \
			$(BASE)/play.hpp \
			$(BASE)/winner.hpp \
			$(BASE)/general_v2.hpp \
			$(BASE)/const.hpp
