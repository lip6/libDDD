lib_LIBRARIES = libDDD.a \
                libDDD_d.a

ddddir      =   $(pkgincludedir)
ddd_hdrs    =   DataSet.h \
                Cache.hh \
                DDD.h \
                DED.h \
                Hom.h \
                MemoryManager.h \
                SDD.h \
                SDED.h \
                SHom.h \
                UniqueTable.h \
                IntDataSet.h \
                hashfunc.hh \
                MLHom.h \
                statistic.hpp \
                process.hpp \
                AdditiveMap.hpp \
                init.hh
ddd_HEADERS =   $(ddd_hdrs)

utildir     =   $(pkgincludedir)/util
util_hdrs   =   util/dotExporter.h \
                util/configuration.hh \
                util/ext_hash_map.hh \
                util/hash_support.hh \
                util/tbb_hash_map.hh \
                util/vector.hh \
                util/set.hh \
                util/map.hh
util_HEADERS =  $(util_hdrs)
           
srcs     =  DED.cpp \
            Cache.hh \
            DDD.cpp \
            Hom.cpp \
            SDED.cpp \
            SDD.cpp \
            SHom.cpp \
            IntDataSet.cpp \
            MLHom.cpp \
            statistic.cpp \
            process.cpp \
            util/dotExporter.cpp

# standard version
libDDD_a_SOURCES    =   $(ddd_hdrs) $(util_hdrs) $(srcs)

# debug version
libDDD_d_a_SOURCES  =   $(ddd_hdrs) $(util_hdrs) $(srcs)
libDDD_d_a_CPPFLAGS =   -g -O0
