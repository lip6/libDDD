noinst_PROGRAMS = morpionv2 morpionvy


DDD_SRCDIR = $(top_srcdir)/src ./
DDD_BUILDDIR = $(top_builddir)/src

BASE = hom

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -I . -g -Wall $(TBBINC_FLAGS)
LDADD = $(DDD_BUILDDIR)/libDDD_d.a

#morpionv1_SOURCES = Hom_Basic.cpp Hom_Basic.hh hom_check_imp.cpp \
#	hom_check_imp.h hom_full.cpp hom_take_cell.cpp morpion.cpp morpion_ctl_v1.cpp \
#	morpion_hom.cpp morpion_hom.hpp

morpionv2_SOURCES = 	$(BASE)/notew.cpp \
			$(BASE)/check_imp.cpp \
			$(BASE)/nowinner.cpp \
			$(BASE)/play.cpp $(BASE)/winner.cpp \
			$(BASE)/general_v2.cpp \
			$(BASE)/check_impossible.cpp \
			$(BASE)/validate_base.cpp \
			morpionv2.cpp

morpionvy_SOURCES = morpionvyann.cpp \
	hom/morpion_hom_vyann.cpp \
	Hom_Basic.hh Hom_Basic.cpp

	
noinst_HEADERS = 	$(BASE)/check_imp.hpp \
			$(BASE)/notew.hpp \
			$(BASE)/nowinner.hpp \
			$(BASE)/play.hpp \
			$(BASE)/take_cell.hpp \
			$(BASE)/winner.hpp \
			$(BASE)/check_impossible.hpp \
			$(BASE)/validate_base.hpp \
			$(BASE)/const.hpp
