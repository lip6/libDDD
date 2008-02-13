#ifndef __EXPRESSION_HOM_HH__
#define __EXPRESSION_HOM_HH__

#include "Hom.h"
#include "IntExpression.hpp"
#include "BoolExpression.hpp"

class Context {
  static std::map<int,Variable> vars;
 public :
  /// Correspondence between variables and integers.
  static int getVariableIndex (const Variable & v);
  /// Return the variable corresponding to an index.
  static const Variable & getVariable (int index);
  /// Set a mapping Variable<->int
  static void setVariableIndex (int index, const Variable &v);
};

/// Assigns the value of expr to variable var.
GHom assignExpr (int var, const IntExpression & expr);

/// Creates a predicate to select paths that verify the expression e.
GHom predicate (const BoolExpression & e);

#endif
