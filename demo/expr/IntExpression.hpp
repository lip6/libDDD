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
  const _IntExpression * concrete;
  friend class _IntExpression;

public :
  // NOT FOR PUBLIC
  IntExpression (const _IntExpression * c); 

  IntExpression (const IntExpression & other);
  
  IntExpression & operator= (const IntExpression & other);
  bool operator== (const IntExpression & other) const ;
  bool operator< (const IntExpression & other) const ;

  ~IntExpression();
  
  typedef std::pair<const Variable&,const IntExpression &> Assignment;
  
  IntExprType getType() const;
  // member print
  void print (std::ostream & os) const ;
  
  // an operator to (partially) resolve expressions.
  // replace occurrences of v (if any) by e.
  const IntExpression & operator& (const Assignment &e) const;



  // basic operators between two expressions.
  // nary
  friend IntExpression operator+(const IntExpression & l,const IntExpression & r) ;
  friend IntExpression operator*(const IntExpression & l,const IntExpression & r) ;

  // binary
  IntExpression operator-(const IntExpression & e) const ;
  IntExpression operator/(const IntExpression & e) const ;
  IntExpression operator%(const IntExpression & e) const ;
  IntExpression operator^(const IntExpression & e) const ;

  IntExpression eval () const ;
  
  
  size_t hash () const;

  // for public convenience
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


};


/******************************************************************************/
namespace __gnu_cxx {
  /// Computes a hash key for a DDD. 
  /// Value returned is based on unicity of concret in unicity table.
  /// Uses D. Knuth's hash function for pointers.  
  template<>
  struct hash<IntExpression> {
    size_t operator()(const IntExpression &g) const{
      return g.hash(); 
    }
  };
}

namespace std {
  /// Compares two DDD in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>
  struct equal_to<IntExpression> {
    bool operator()(const IntExpression &g1,const IntExpression &g2) const{
      return g1==g2;
    }
  };
}

namespace std {
  /// Compares two DDD in hash tables. 
  /// Value returned is based on unicity of concret in unicity table.
  template<>
  struct less<IntExpression> {
    bool operator()(const IntExpression &g1,const IntExpression &g2) const{
      return g1<g2;
    }
  };
}


#endif
