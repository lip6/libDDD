#ifndef __INT_EXPRESSION_HH__
#define __INT_EXPRESSION_HH__

#include <set>
#include <iostream>
#include <string>
#include <UniqueTable.h>
#include "Variable.hpp"


typedef enum { 
  CONST, // an integer constant
  VAR,  // a variable
  PLUS, // nary add
  MULT,  // nary multiplication
  MINUS, // binary minus
  DIV,  // binary division
  MOD, // binary modulo
  POW // binary power of
} IntExprType ;
//  NEG  // unary neg


class _IntExpression ;

// immutable class for handling integer expressions.
class IntExpression {
public :
  virtual ~IntExpression() {};
  
  typedef std::pair<const Variable&,const IntExpression &> Assignment;
  
  virtual IntExprType getType() const =0;
  // member print
  virtual void print (std::ostream & os) const = 0;
  
  // an operator to (partially) resolve expressions.
  // replace occurrences of v (if any) by e.
  const IntExpression & operator& (const Assignment &e) const;
  // basic operators between two expressions.
  // binary
  const IntExpression & operator-(const IntExpression & e) const ;
  const IntExpression & operator/(const IntExpression & e) const ;
  const IntExpression & operator%(const IntExpression & e) const ;
  const IntExpression & operator^(const IntExpression & e) const ;

  virtual const IntExpression & eval () const = 0 ;

  // for public convenience
  friend std::ostream & operator<< (std::ostream & os, const IntExpression & e);
};

// nary
// overloaded in Plus class
const IntExpression & operator+(const IntExpression & l,const IntExpression & r) ;
const IntExpression & operator*(const IntExpression & l,const IntExpression & r) ;


typedef std::multiset<const IntExpression *> NaryParamType ;

class IntExpressionFactory {
  static UniqueTable<IntExpression> unique;
public :
  static const IntExpression * createNary (IntExprType type, NaryParamType params) ;
  static const IntExpression * createBinary (IntExprType type, const IntExpression * l, const IntExpression * r) ;
  static const IntExpression * createConstant (int v);
  static const IntExpression * createVariable (const Variable & v) ;


};




#endif
