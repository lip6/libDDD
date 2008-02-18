
noinst_PROGRAMS = testExpr testML testMLExpr

DDD_SRCDIR = $(top_srcdir)/src
DDD_BUILDDIR = $(top_builddir)/src

AM_CPPFLAGS =  -I $(DDD_SRCDIR) -g -Wall
LDADD = $(DDD_BUILDDIR)/libDDD_d.a
if PARALLEL
AM_LDFLAGS = -ltbb
endif


EXPR_SRCS = IntExpression.hpp IntExpression.cpp BoolExpression.hpp BoolExpression.cpp

testExpr_SOURCES = $(EXPR_SRCS) main.cpp

testML_SOURCES = mainML.cpp

testMLExpr_SOURCES =  $(EXPR_SRCS) mainMLexpr.cpp ExprHom.cpp ExprHom.hpp