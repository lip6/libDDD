#include "ExprHom.hpp"
#include "MLHom.h"
#include <cassert>

// predeclarations
MLHom queryExpression (const IntExpression & e);

GHom assertion (const Assertion & a);


std::map<int,Variable> Context::vars = std::map<int,Variable> ();

void Context::setVariableIndex ( int index, const Variable &v) {
  vars.insert(std::make_pair(index,v));
}

const Variable & Context::getVariable (int index) {
  std::map<int,Variable>::const_iterator it = vars.find(index);
  if (it != vars.end() )
    return it->second;
  else
    throw "Unknown variable !!";
}


// assign an expression to a variable
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
      && ! expr.isSupport(Context::getVariable(var));
  }

  GHom phi(int vr, int vl) const {
    IntExpression e = expr ;
    if (expr.isSupport(Context::getVariable(vr))) {
      e = e & IntExpressionFactory::createAssertion(Context::getVariable(vr),IntExpressionFactory::createConstant(vl));
    }
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

  bool
  skip_variable(int var) const
  {
    return ! b.isSupport(Context::getVariable(var));
  }


  HomHomMap phi (int var,int val) const {
    IntExpression e = b ;
    if (e.isSupport(Context::getVariable(var))) {
      e = e & IntExpressionFactory::createAssertion(Context::getVariable(var),IntExpressionFactory::createConstant(val));
    }
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
    return ! ass.isSupport(Context::getVariable(var));
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
    return ! expr.isSupport(Context::getVariable(var)) ;
  }
  
  GHom phi(int vr, int vl) const {
    BoolExpression e = expr ;
    if (expr.isSupport(Context::getVariable(vr))) {
      e = e & IntExpressionFactory::createAssertion(Context::getVariable(vr),IntExpressionFactory::createConstant(vl));
    }
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
