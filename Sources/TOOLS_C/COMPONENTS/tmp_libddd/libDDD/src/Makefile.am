lib_LIBRARIES = libDDD.a libDDD_i.a libDDD_d.a libDDD_p.a 


SRCS = DED.cpp DDD.cpp Hom.cpp SDED.cpp SDD.cpp SHom.cpp \
   DataSet.h  DED.h  MemoryManager.h  SDED.h  UniqueTable.h \
   DDD.h	   Hom.h  SDD.h		   SHom.h

# default options => -o2 
libDDD_a_SOURCES = $(SRCS)
libDDD_a_CPPFLAGS = -O2

# STL instantiated version for hash profiling
libDDD_i_a_SOURCES = $(SRCS)
libDDD_i_a_CPPFLAGS = -D INST_STL -isystem  $(top_srcdir)/STLProf

# change options to debug 
libDDD_d_a_SOURCES = $(SRCS)
libDDD_d_a_CPPFLAGS = -g

# profiling version : -pg
libDDD_p_a_SOURCES = $(SRCS)
libDDD_p_a_CPPFLAGS = -g -D NDEBUG  -pg


