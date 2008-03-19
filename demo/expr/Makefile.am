noinst_PROGRAMS = testExpr testML testMLExpr

# Flags for TBB
if WITH_LIBTBBINC_PATH
TBBINC_FLAGS=-I $(LIBTBB_INC)
endif

if WITH_LIBTBBBIN_PATH
TBBBIN_FLAGS=-L $(LIBTBB_BIN)
endif

DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall  $(TBBINC_FLAGS)
LDADD = $(DDD_BUILDDIR)/libDDD_d.a

if REENTRANT
    AM_LDFLAGS = -ltbb $(TBBBIN_FLAGS)
endif

EXPR_SRCS = IntExpression.hpp \
            IntExpression.cpp \
            BoolExpression.hpp \
            BoolExpression.cpp \
            Variable.hpp

testExpr_SOURCES =  $(EXPR_SRCS) \
                    main.cpp

testML_SOURCES  = mainML.cpp

testMLExpr_SOURCES =    $(EXPR_SRCS) \
                        mainMLexpr.cpp \
                        ExprHom.cpp \
                        ExprHom.hpp
