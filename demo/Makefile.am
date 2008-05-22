SUBDIRS = hanoi expr

noinst_PROGRAMS = tst1 tst2 tst3 tst4 tst5 tst6 tst7 tst8 tst9 tst10 tst11 tst12 #tst13

# Flags for TBB
if WITH_LIBTBBINC_PATH
TBBINC_FLAGS=-I $(LIBTBB_INC)
endif

if WITH_LIBTBBBIN_PATH
TBBBIN_FLAGS=-L $(LIBTBB_BIN)
endif

DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall $(TBBINC_FLAGS)
LDADD = $(DDD_BUILDDIR)/libDDD_d.a
if REENTRANT
AM_LDFLAGS = -ltbb $(TBBBIN_FLAGS)
endif

PLUSPLUS = PlusPlus.hh PlusPlus.cpp
SETVAR = SetVar.hh SetVar.cpp
SWAPVAR = PermuteVar.hh PermuteVar.cpp

tst1_SOURCES = tst1.cpp
tst2_SOURCES = tst2.cpp
tst3_SOURCES = tst3.cpp $(PLUSPLUS)
tst4_SOURCES = tst4.cpp $(PLUSPLUS)
tst5_SOURCES = tst5.cpp $(SETVAR)
tst6_SOURCES = tst6.cpp $(SWAPVAR)
tst7_SOURCES = tst7.cpp
tst8_SOURCES = tst8.cpp
tst9_SOURCES = tst9.cpp
tst10_SOURCES = tst10.cpp $(PLUSPLUS)
tst11_SOURCES = tst11.cpp
tst12_SOURCES = tst12.cpp 
#tst13_SOURCES = tst13.cpp 
#tst13_LDADD =  $(DDD_BUILDDIR)/libDDD_ev.a
#tst13_CPPFLAGS = -I $(DDD_SRCDIR) -g -Wall -D EVDDD
# tst2_CPPFLAGS = 




