lib_LIBRARIES = libDDD.a  libDDD_d.a libDDD_p.a 
# libDDD_i.a  BROKEN WITH GCC 3.4
GMPLIBDIR = /home/thierry/PNDDD/RdPDDD/gmp-4.1.2/.libs
GMPINCDIR = /home/thierry/PNDDD/RdPDDD/gmp-4.1.2

SRCS = DED.cpp DDD.cpp Hom.cpp SDED.cpp SDD.cpp SHom.cpp \
   DataSet.h  DED.h  MemoryManager.h  SDED.h  UniqueTable.h \
   DDD.h	   Hom.h  SDD.h		   SHom.h

AM_CPPFLAGS =  -I$(GMPINCDIR)
LDADD = -L$(GMPLIBDIR) -lgmp -lgmpxx

# default options => -o2 
libDDD_a_SOURCES = $(SRCS)
libDDD_a_CPPFLAGS = -O2 -I$(GMPINCDIR)

# STL instantiated version for hash profiling
libDDD_i_a_SOURCES = $(SRCS)
libDDD_i_a_CPPFLAGS = -D INST_STL -isystem  $(top_srcdir)/STLProf -I$(GMPINCDIR)

# change options to debug 
libDDD_d_a_SOURCES = $(SRCS)
libDDD_d_a_CPPFLAGS = -g -I$(GMPINCDIR)

# profiling version : -pg
libDDD_p_a_SOURCES = $(SRCS)
libDDD_p_a_CPPFLAGS = -g -D NDEBUG  -pg -I$(GMPINCDIR)


