SUBDIRS = hanoi expr

noinst_PROGRAMS = tst1 tst2 tst3 tst4 tst5 tst6 tst7 tst8 tst9 tst10 tst11 tst12 #tst13


DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall
LDADD = $(DDD_BUILDDIR)/libDDD_d.a
if REENTRANT
AM_LDFLAGS = -ltbb
endif


tst1_SOURCES = tst1.cpp
tst2_SOURCES = tst2.cpp
tst3_SOURCES = tst3.cpp
tst4_SOURCES = tst4.cpp
tst5_SOURCES = tst5.cpp
tst6_SOURCES = tst6.cpp
tst7_SOURCES = tst7.cpp
tst8_SOURCES = tst8.cpp
tst9_SOURCES = tst9.cpp
tst10_SOURCES = tst10.cpp
tst11_SOURCES = tst11.cpp
tst12_SOURCES = tst12.cpp 
#tst13_SOURCES = tst13.cpp 
#tst13_LDADD =  $(DDD_BUILDDIR)/libDDD_ev.a
#tst13_CPPFLAGS = -I $(DDD_SRCDIR) -g -Wall -D EVDDD
# tst2_CPPFLAGS = 




