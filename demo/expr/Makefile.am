
noinst_PROGRAMS = testExpr testML testMLExpr

DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall
LDADD = $(DDD_BUILDDIR)/libDDD_d.a
# AM_LDFLAGS = -ltbb

# AM_CPPFLAGS = -D INST_STL -I $(DDD_SRCDIR)
# LDADD = $(DDD_BUILDDIR)/libDDD_i.a

EXPR_SRCS = IntExpression.hpp IntExpression.cpp BoolExpression.hpp BoolExpression.cpp

testExpr_SOURCES = $(EXPR_SRCS) main.cpp

testML_SOURCES = mainML.cpp

testMLExpr_SOURCES =  $(EXPR_SRCS) mainMLexpr.cpp ExprHom.cpp ExprHom.hpp