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

class Assertion;

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

  IntExpression (int cst);
  IntExpression (const Variable &var);
  // copy constructor
  IntExpression (const IntExpression & other);
  // assignment
  IntExpression & operator= (const IntExpression & other);
  // destructor (does not reclaim memory). Use IntExpressionFactory::garbage for that.
  ~IntExpression();


  // comparisons for set/hash storage (based on unique property of concrete).
  // uses equals and less because operator == and < are overloaded to produce BoolExpression
  bool equals (const IntExpression & other) const ;
  bool less (const IntExpression & other) const ;
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

  IntExpression operator& (const Assertion &a) const;

  // resolve what can be resolved at this stage. 
  // Result is a constant expression iff. the expression has no more variables.
  IntExpression eval () const ;

  /// only valid for CONST expressions
  /// use this call only in form : if (e.getType() == CONST) { int j = e.getValue() ; ...etc }
  /// Exceptions will be thrown otherwise.
  int getValue () const ;

  /// To determine whether a given variable is mentioned in an expression.
  bool isSupport (const Variable & v) const;

  // for pretty print
  friend std::ostream & operator<< (std::ostream & os, const IntExpression & e);
};


class Assertion {
  std::pair<IntExpression,IntExpression> mapping ;
public :
  Assertion (const IntExpression & var, const IntExpression & val);
  IntExpression getValue (const IntExpression & v) const ;

  Assertion operator & (const Assertion & other) const;
  
  /// To determine whether a given variable is mentioned in an expression.
  bool isSupport (const Variable & v) const;

  size_t hash() const;
  bool operator== (const Assertion & ) const ;
};


typedef std::multiset<IntExpression> NaryParamType ;

class IntExpressionFactory {
  static UniqueTable<_IntExpression> unique;
public :
  static IntExpression  createNary (IntExprType type, NaryParamType params) ;
  static IntExpression  createBinary (IntExprType type, const IntExpression & l, const IntExpression & r) ;
  static IntExpression  createConstant (int v);
  static IntExpression  createVariable (const Variable & v) ;
  
  static Assertion createAssertion (const Variable & v,const IntExpression & e);
  static Assertion createAssertion (const IntExpression & v,const IntExpression & e);

  static void printStats (std::ostream &os);

  // following administrative functions are not really for public usage.
  static const _IntExpression * createUnique(const _IntExpression &);
  static void destroy (_IntExpression * e);
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
      return g1.equals(g2);
    }
  };
}

namespace std {
  template<>
  struct less<IntExpression> {
    bool operator()(const IntExpression &g1,const IntExpression &g2) const{
      return g1.less(g2);
    }
  };
}


#endif
