lib_LIBRARIES = libDDD.a \
                libDDD_d.a

MYSRCDIR= $(abs_top_srcdir)/src

ddddir      =   $(pkgincludedir)
ddd_hdrs    =   DataSet.h \
                Cache.hh \
                DDD.h \
                DED.h \
                FixObserver.hh \
                Hom.h \
                Hom_Basic.hh \
                MemoryManager.h \
                MLCache.hh \
                SDD.h \
                SDED.h \
                SHom.h \
                UniqueTable.h \
		UniqueTableId.hh \
                IntDataSet.h \
                hashfunc.hh \
                MLHom.h \
                MLSHom.h \
                statistic.hpp \
                process.hpp \
                AdditiveMap.hpp \
                init.hh

utildir     =   $(pkgincludedir)/util
util_hdrs   =   util/dotExporter.h \
                util/configuration.hh \
                util/ext_hash_map.hh \
                util/hash_support.hh \
		util/hash_set.hh \
                util/tbb_hash_map.hh \
                util/vector.hh \
                util/set.hh \
                util/map.hh \
		google/sparse_hash_map \
		google/sparse_hash_set \
		google/sparsetable \
		google/template_util.h \
		google/type_traits.h \
		google/sparsehash/densehashtable.h  \
		google/sparsehash/hashtable-common.h   \
		google/sparsehash/libc_allocator_with_realloc.h   \
		google/sparsehash/sparseconfig.h   \
		google/sparsehash/sparsehashtable.h

EXTRA_DIST = google/AUTHORS  google/COPYING  google/README

nobase_ddd_HEADERS =   $(ddd_hdrs) $(util_hdrs)

srcs     =  DED.cpp \
            Cache.hh \
            DDD.cpp \
            FixObserver.cpp \
            Hom.cpp \
            Hom_Basic.cpp \
            SDED.cpp \
            SDD.cpp \
            SHom.cpp \
            IntDataSet.cpp \
            MLHom.cpp \
            MLSHom.cpp \
            statistic.cpp \
            process.cpp \
            util/dotExporter.cpp

# Flags for TBB
if WITH_LIBTBBINC_PATH
TBBINC_FLAGS=-I $(LIBTBB_INC)
endif


# standard version
libDDD_a_SOURCES    =   $(ddd_hdrs) $(util_hdrs) $(srcs)
libDDD_a_CPPFLAGS =   -g -O3 -Wall -DNDEBUG $(TBBINC_FLAGS) -I $(MYSRCDIR)

# debug version
libDDD_d_a_SOURCES  =   $(ddd_hdrs) $(util_hdrs) $(srcs)
libDDD_d_a_CPPFLAGS =   -g -O0 -Wall $(TBBINC_FLAGS) -I $(MYSRCDIR)
