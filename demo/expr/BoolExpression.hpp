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


class _BoolExpression ;

// immutable class for handling boolean expressions.
class BoolExpression {
    // concrete storage
  const _BoolExpression * concrete;
  // access to concrete
  friend class _BoolExpression;
  friend class BoolConstExpr;
  friend class BoolExpressionFactory;

  // For factory use
  BoolExpression (const _BoolExpression * c); 

public :

  // copy constructor
  BoolExpression (const BoolExpression & other);
  // assignment
  BoolExpression & operator= (const BoolExpression & other);
  // destructor (does not reclaim memory). Use BoolExpressionFactory::garbage for that.
  ~BoolExpression();

  // comparisons for set/hash storage (based on unique property of concrete).
  bool operator== (const BoolExpression & other) const ;
  bool operator< (const BoolExpression & other) const ;
  size_t hash () const;
  
  BoolExprType getType() const ;
  // member print
  void print (std::ostream & os) const ;
  
  // an operator to (partially) resolve expressions.
  // replace occurrences of v (if any) by e.
  BoolExpression operator& (const Assertion &a) const;
  // basic operators between two expressions.
  // and
  friend BoolExpression operator&&(const BoolExpression & l,const BoolExpression & r);
  // or
  friend BoolExpression operator||(const BoolExpression & l,const BoolExpression & r);
  // not
  BoolExpression operator! () const ;

  // resolve what can be resolved at this stage. 
  // Result is a constant expression iff. the expression has no more variables.
  BoolExpression eval () const ;

  /// only valid for CONST expressions
  /// use this call only in form : if (e.getType() == BOOLCONST) { int j = e.getValue() ; ...etc }
  /// Exceptions will be thrown otherwise.
  bool getValue () const ;

  // for public convenience
  friend std::ostream & operator<< (std::ostream & os, const BoolExpression & e);
};

// binary comparisons
BoolExpression  operator==(const IntExpression & l, const IntExpression & r) ;
BoolExpression  operator!=(const IntExpression & l, const IntExpression & r) ;
BoolExpression  operator< (const IntExpression & l, const IntExpression & r) ;
BoolExpression  operator> (const IntExpression & l, const IntExpression & r) ;
BoolExpression  operator>=(const IntExpression & l, const IntExpression & r) ;
BoolExpression  operator<=(const IntExpression & l, const IntExpression & r) ;


typedef std::set<BoolExpression> NaryBoolParamType ;

class BoolExpressionFactory {
  static UniqueTable<_BoolExpression> unique;
public :
  // and and or of boolean expressions
  static BoolExpression createNary (BoolExprType type, const NaryBoolParamType &params) ;
  // not !
  static BoolExpression createNot  (const BoolExpression & e) ;
  // a boolean constant T or F
  static BoolExpression createConstant (bool b);

  // a comparison (==,!=,<,>,<=,>=) between two integer expressions
  static BoolExpression createComparison (BoolExprType type, const IntExpression & l, const IntExpression & r) ;

  static void destroy (_BoolExpression * e);
  static void printStats (std::ostream &os);
};


/**************  administrative trivia. *********************************/
/******************************************************************************/
namespace __gnu_cxx { 
  template<>
  struct hash<BoolExpression> {
    size_t operator()(const BoolExpression &g) const{
      return g.hash(); 
    }
  };
}

namespace std {
  template<>
  struct equal_to<BoolExpression> {
    bool operator()(const BoolExpression &g1,const BoolExpression &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  template<>
  struct less<BoolExpression> {
    bool operator()(const BoolExpression &g1,const BoolExpression &g2) const{
      return g1<g2;
    }
  };
}


#endif
