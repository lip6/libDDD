#include "MLHom.h"
#include "IntExpression.hpp"
#include "BoolExpression.hpp"
#include <iostream>
#include <cassert>
using namespace std;


typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

const Variable globalVariables[] = {Variable("A"),Variable("B"),Variable("C"),Variable("D"),Variable("E"),Variable("F"),Variable("G")};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}


// predeclarations
GHom assignExpr (int var, const IntExpression & val);
MLHom queryExpression (const IntExpression & e);
GHom assertion (const Assertion & a);
GHom predicate (const BoolExpression & e);

class _AssertionHom;

// assign a constant to a value
class _AssignExpr:public StrongHom {
  int var;
  IntExpression expr;
public:
  _AssignExpr(int varr, const IntExpression & e) : var(varr), expr(e) {}
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   

  bool
  skip_variable(int var) const
  {
    return var != this->var 
      && false // ! expr.contains(var) : add API to expression to get support variables
      ;
  }

  GHom phi(int vr, int vl) const {
    IntExpression e = expr ;
    //    if (expr.contains(globalVariables[vr])) {
    e = e & IntExpressionFactory::createAssertion(globalVariables[vr],IntExpressionFactory::createConstant(vl));
      //    }
    e = e.eval();
    if (vr == var) {
      if (e.getType() == CONST) {
	// Constant :
	return GHom(var, e.getValue());
      } else {
      // still need to resolve.
	return MLHom(assignExpr(var,e),MLHom(vr,vl,queryExpression(e)));
      }
    } else {
      return GHom(vr,vl, assignExpr(var,e));
    }
  }
  size_t hash() const {
    return 6619*var^expr.hash();
  }
  bool operator==(const StrongHom &s) const {
    _AssignExpr* ps = (_AssignExpr*)&s;
    return var == ps->var && expr.equals(ps->expr);
  }

  GHom compose (const GHom & other) const ;
};

GHom assignExpr (int var,const IntExpression & val) {
  return new _AssignExpr(var,val);
}

// a MLHom to handle : a =? b
// initialized by querying for a =? a and resolve until right hand side is constant
class _QueryMLHom : public StrongMLHom {
  IntExpression a;
  IntExpression b;
public :
  _QueryMLHom (const IntExpression & aa, const IntExpression & bb) : a(aa),b(bb) {}

  HomHomMap phi (int var,int val) const {
    IntExpression e = b ;
    //    if (expr.contains(globalVariables[vr])) {
    e = e & IntExpressionFactory::createAssertion(globalVariables[var],IntExpressionFactory::createConstant(val));
    //}
    e= e.eval();


    GHom homup = assertion(IntExpressionFactory::createAssertion(a,e));
    MLHom homdown = MLHom::id;
    
    if (e.getType() == CONST) {
      // Constant :
      homdown = GHom(var,val,GHom::id);
    } else {
      homdown = MLHom (var,val,queryExpression(e));
    }
    HomHomMap ret;
    ret.add(homup,homdown);
    return ret;
  }

  bool operator== (const StrongMLHom & s) const {
    _QueryMLHom * ps = (_QueryMLHom *)&s;
    return a.equals(ps->a) && b.equals(ps->b);    
  }

  size_t hash() const {
    return 7489*(a.hash()^(b.hash()+1));
  }


};

MLHom queryExpression (const IntExpression & a) {
  return new _QueryMLHom(a,a);
}

// perform varl := varr independently of variable ordering.
class _AssertionHom:public StrongHom {
  Assertion ass;
public:
  _AssertionHom(const Assertion & expr) : ass(expr) {}
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   

  bool
  skip_variable(int var) const
  {
    return false;
  }

  Assertion getAssertion () const { return ass;}

  GHom phi(int vr, int vl) const {
    assert(false);
    return GDDD::null;
  }
  size_t hash() const {
    return 7717*ass.hash();
  }
  bool operator==(const StrongHom &s) const {
    _AssertionHom* ps = (_AssertionHom*)&s;
    return ass == ps->ass;
  }

  GHom compose (const GHom & other) const {
    const _GHom * c = get_concret(other);
    if (typeid(*c) == typeid(_AssertionHom)) {
      return new _AssertionHom(ass &  ((const _AssertionHom *)c)->getAssertion());
    } else {
      return _GHom::compose(other);
    }
  }
};

GHom assertion (const Assertion & e) {
  return new _AssertionHom(e);
}

GHom _AssignExpr::compose (const GHom & other) const {
  const _GHom * c = get_concret(other);
  if (typeid(*c) == typeid(_AssertionHom)) {
    return assignExpr(var,(expr & ((const _AssertionHom *)c)->getAssertion()).eval());
  } else {
    return _GHom::compose(other);
  }
}


class _Predicate:public StrongHom {
  BoolExpression expr;
public:
  _Predicate(const BoolExpression & e) : expr(e) {}
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   

  bool
  skip_variable(int var) const
  {
    return false // ! expr.contains(var) : add API to expression to get support variables
      ;
  }
  
  GHom phi(int vr, int vl) const {
    BoolExpression e = expr ;
    //    if (expr.contains(globalVariables[vr])) {
    e = e & IntExpressionFactory::createAssertion(globalVariables[vr],IntExpressionFactory::createConstant(vl));
    //    }
    e = e.eval();
    if (e.getType() == BOOLCONST) {
      // Constant :
      if (e.getValue()) 
	return GHom(vr,vl);
      else
	return GDDD::null;
    } else {
      // still need to resolve.
      return GHom(vr,vl, predicate(e));
    }
  }
  size_t hash() const {
    return 16363*expr.hash();
  }
  bool operator==(const StrongHom &s) const {
    _Predicate* ps = (_Predicate*)&s;
    return expr == ps->expr;
  }

  GHom compose (const GHom & other) const ;
};

GHom predicate (const BoolExpression & e) {
  return new _Predicate(e);
}

GHom _Predicate::compose (const GHom & other) const {
  const _GHom * c = get_concret(other);
  if (typeid(*c) == typeid(_AssertionHom)) {
    return predicate((expr & ((const _AssertionHom *)c)->getAssertion()).eval());
  } else {
    return _GHom::compose(other);
  }
}





int main () {
  initName();

  DDD test1 = GDDD(A,1,GDDD(B,2,GDDD(C,3,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test2 = GDDD(A,2,GDDD(B,2,GDDD(C,3,GDDD(D,5,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test3 = GDDD(A,1,GDDD(B,2,GDDD(C,12,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,9)))))));
  DDD test4 = GDDD(A,1,GDDD(B,2,GDDD(C,12,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test5 = test1 + test2 + test3 + test4;

  IntExpression Aexpr = IntExpressionFactory::createVariable(globalVariables[A]);
  IntExpression Bexpr = IntExpressionFactory::createVariable(globalVariables[B]);
  IntExpression Eexpr = IntExpressionFactory::createVariable(globalVariables[E]);
  IntExpression Gexpr = IntExpressionFactory::createVariable(globalVariables[G]);
  

  GHom be = assignExpr(B,Eexpr);
  GHom eb = assignExpr(E,Bexpr);
  GHom bg = assignExpr(B,Gexpr);

  IntExpression eplusg = Eexpr + Gexpr + Bexpr + Aexpr;
  GHom beg = assignExpr(B,eplusg);  

  cout << "Input :\n" << test5 << endl;
  cout << "b:=" << Eexpr << " \n" << be(test5) << endl;
  cout << "e:=" << Bexpr << " \n" << eb(test5) << endl;
  cout << "b:=" << Gexpr << " \n" << bg(test5) << endl;
  cout << "b:=" << eplusg << " \n" <<  beg(test5) << endl;

  GHom gmax = predicate (Gexpr < 9);
  GHom incrG = assignExpr(G, Gexpr+1);

  cout <<  "g:=" <<  (Gexpr+1) << "["<<  (Gexpr < 9) << "]" << endl;
  cout << (incrG & gmax) (test5) << endl ;
  cout << "fixpoint :" << endl;
  cout << fixpoint((incrG & gmax) + GHom::id) (test5)<< endl;

  return 0;
}
