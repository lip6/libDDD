noinst_PROGRAMS =   hanoi_v1 \
                    hanoi_v1_bis \
                    hanoi_v1_ter \
                    hanoi_v2 \
                    hanoi_v3 \
                    hanoi_v4 \
                    hanoi_v5 \
                    hanoi_v6 \
                    hanoi_v7 \
                    hanoi_v8 \
                    hanoi_v9

DDD_SRCDIR      =   $(top_srcdir)/src
DDD_BUILDDIR    =   $(top_builddir)/src

# Flags for TBB
if WITH_LIBTBBINC_PATH
TBBINC_FLAGS=-I $(LIBTBB_INC)
endif

if WITH_LIBTBBBIN_PATH
TBBBIN_FLAGS=-L $(LIBTBB_BIN)
endif

AM_CPPFLAGS     =  -I $(DDD_SRCDIR) -Wall -Wextra -O3 $(TBBINC_FLAGS)

LDADD           =   $(DDD_BUILDDIR)/libDDD.a

if REENTRANT
    AM_LDFLAGS = -ltbb $(TBBBIN_FLAGS)
endif

hanoi_v1_SOURCES = hanoi_v1.cpp
hanoi_v1_bis_SOURCES = hanoi_v1_bis.cpp
hanoi_v1_ter_SOURCES = hanoi_v1_ter.cpp
hanoi_v2_SOURCES = hanoi_v2.cpp
hanoi_v3_SOURCES = hanoi_v3.cpp
hanoi_v4_SOURCES = hanoi_v4.cpp
hanoi_v5_SOURCES = hanoi_v5.cpp
hanoi_v6_SOURCES = hanoi_v6.cpp
hanoi_v7_SOURCES = hanoi_v7.cpp
hanoi_v8_SOURCES = hanoi_v8.cpp
hanoi_v9_SOURCES = hanoi_v9.cpp

