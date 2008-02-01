#ifndef __BOOL_EXPRESSION_HH__
#define __BOOL_EXPRESSION_HH__

#include <set>
#include <iostream>
#include <string>
#include <UniqueTable.h>
#include "IntExpression.hpp"
#include "Variable.hpp"


typedef enum { 
  BOOLCONST, // a boolean constant true or false
  BOOLVAR,  // a variable
  OR, // nary or
  AND,  // nary and
  NOT, // unary not
  EQ,  // ==
  NEQ, // !=
  LT, // <
  GT, // >
  LEQ, // <=
  GEQ  // >=
} BoolExprType ;


// immutable class for handling boolean expressions.
class BoolExpression {
public :
  virtual ~BoolExpression() {};
  
  typedef std::pair<const Variable&,const BoolExpression &> Assignment;
  
  virtual BoolExprType getType() const =0;
  // member print
  virtual void print (std::ostream & os) const = 0;
  
  // an operator to (partially) resolve expressions.
  // replace occurrences of v (if any) by e.
  const BoolExpression & operator& (const Assignment &e) const;
  // basic operators between two expressions.
  // and
  virtual const BoolExpression & operator&&(const BoolExpression & e) const ;
  // or
  virtual const BoolExpression & operator||(const BoolExpression & e) const ;
  // not
  virtual const BoolExpression & operator! () const ;

  // for public convenience
  friend std::ostream & operator<< (std::ostream & os, const BoolExpression & e);
};

// binary comparisons
const BoolExpression & operator==(const IntExpression & l, const IntExpression & r) ;
const BoolExpression & operator!=(const IntExpression & l, const IntExpression & r) ;
const BoolExpression & operator< (const IntExpression & l, const IntExpression & r) ;
const BoolExpression & operator> (const IntExpression & l, const IntExpression & r) ;
const BoolExpression & operator>=(const IntExpression & l, const IntExpression & r) ;
const BoolExpression & operator<=(const IntExpression & l, const IntExpression & r) ;


typedef std::set<const BoolExpression *> NaryBoolParamType ;

class BoolExpressionFactory {
  static UniqueTable<BoolExpression> unique;
public :
  // and and or of boolean expressions
  static const BoolExpression * createNary (BoolExprType type, NaryBoolParamType params) ;
  // not !
  static const BoolExpression * createNot  (const BoolExpression * e) ;
  // a boolean constant T or F
  static const BoolExpression * createConstant (bool b);
  // a boolean variable
  static const BoolExpression * createVariable (const Variable & v) ;

  // a comparison (==,!=,<,>,<=,>=) between two integer expressions
  static const BoolExpression * createComparison (BoolExprType type, const IntExpression * l, const IntExpression * r) ;
};

#endif
