lib_LIBRARIES = libDDD.a  libDDD_d.a libDDD_otfg.a

# libDDD_p.a  DEACTIVATED BUILD
# libDDD_i.a  BROKEN WITH GCC 3.4

SRCS = DED.cpp DDD.cpp Hom.cpp SDED.cpp SDD.cpp SHom.cpp \
   DataSet.h  DED.h  MemoryManager.h  SDED.h  UniqueTable.h \
   DDD.h	   Hom.h  SDD.h		   SHom.h

AM_CPPFLAGS =  -WAll 

# default options => -o2 
libDDD_a_SOURCES = $(SRCS)
libDDD_a_CPPFLAGS = -O2  -WAll 

# STL instantiated version for hash profiling
# libDDD_i_a_SOURCES = $(SRCS)
# libDDD_i_a_CPPFLAGS = -D INST_STL -isystem  $(top_srcdir)/STLProf 

# change options to debug 
libDDD_d_a_SOURCES = $(SRCS)
libDDD_d_a_CPPFLAGS = -g  -WAll 

# profiling version : -pg
# libDDD_p_a_SOURCES = $(SRCS)
# libDDD_p_a_CPPFLAGS = -g -D NDEBUG  -pg 

# change options to activate OTF garbage collection (reference counting on GSDD) 
libDDD_otfg_a_SOURCES = $(SRCS)
libDDD_otfg_a_CPPFLAGS = -g -O2 -WAll -D OTF_GARBAGE
