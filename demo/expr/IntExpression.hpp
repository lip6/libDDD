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

class _IntExpression ;

// A class to handle integer expressions.
// Relies on concrete a pointer into unique table.
// Use factory to build instances.
class IntExpression {
  // concrete storage
  const _IntExpression * concrete;
  // access to concrete
  friend class _IntExpression;
  // access to _IntExpression* constructor
  friend class VarExpr;
  friend class ConstExpr;
  friend class IntExpressionFactory;

  // For factory use
  IntExpression (const _IntExpression * c); 
public :

  // copy constructor
  IntExpression (const IntExpression & other);
  // assignment
  IntExpression & operator= (const IntExpression & other);
  // destructor (does not reclaim memory). Use IntExpressionFactory::garbage for that.
  ~IntExpression();


  // comparisons for set/hash storage (based on unique property of concrete).
  bool operator== (const IntExpression & other) const ;
  bool operator< (const IntExpression & other) const ;
  size_t hash () const;
  
  // return the type of an expression.
  IntExprType getType() const;
  // member print
  void print (std::ostream & os) const ;
  
  // basic operators between two expressions.
  // nary
  friend IntExpression operator+(const IntExpression & l,const IntExpression & r) ;
  friend IntExpression operator*(const IntExpression & l,const IntExpression & r) ;

  // binary
  IntExpression operator-(const IntExpression & e) const ;
  IntExpression operator/(const IntExpression & e) const ;
  IntExpression operator%(const IntExpression & e) const ;
  IntExpression operator^(const IntExpression & e) const ;


  // an operator to (partially) resolve expressions.
  // replace occurrences of v (if any) by e.
  typedef std::pair<const Variable&,const IntExpression &> Assignment;
  const IntExpression & operator& (const Assignment &e) const;

  // resolve what can be resolved at this stage. 
  // Result is a constant expression iff. the expression has no more variables.
  IntExpression eval () const ;

  // for pretty print
  friend std::ostream & operator<< (std::ostream & os, const IntExpression & e);
};


typedef std::multiset<IntExpression> NaryParamType ;

class IntExpressionFactory {
  static UniqueTable<_IntExpression> unique;
public :
  static IntExpression  createNary (IntExprType type, NaryParamType params) ;
  static IntExpression  createBinary (IntExprType type, const IntExpression & l, const IntExpression & r) ;
  static IntExpression  createConstant (int v);
  static IntExpression  createVariable (const Variable & v) ;

  static void garbage() ;
  static void printStats (std::ostream &os);
};


/**************  administrative trivia. *********************************/
/******************************************************************************/
namespace __gnu_cxx { 
  template<>
  struct hash<IntExpression> {
    size_t operator()(const IntExpression &g) const{
      return g.hash(); 
    }
  };
}

namespace std {
  template<>
  struct equal_to<IntExpression> {
    bool operator()(const IntExpression &g1,const IntExpression &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  template<>
  struct less<IntExpression> {
    bool operator()(const IntExpression &g1,const IntExpression &g2) const{
      return g1<g2;
    }
  };
}


#endif
