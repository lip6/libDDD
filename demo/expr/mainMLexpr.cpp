#include "MLHom.h"
#include "IntExpression.hpp"
#include <iostream>
using namespace std;


typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

const Variable globalVariables[] = {Variable("A"),Variable("B"),Variable("C"),Variable("D"),Variable("E"),Variable("F"),Variable("G")};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}



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
    if (e.getType() == CONST) {
      // Constant :
      return GHom(var, e.getValue());
    } else {
      // still need to resolve.
      //      return MLHom(AssignExpr(var,e),)
      return GHom::id;
    }
  }
  size_t hash() const {
    return 6619*var^expr.hash();
  }
  bool operator==(const StrongHom &s) const {
    _AssignExpr* ps = (_AssignExpr*)&s;
    return var == ps->var && expr == ps->expr;
  }
};

GHom AssignExpr (int var, IntExpression val) {
  return new _AssignExpr(var,val);
}

// a MLHom to handle : a := b
// seeks value of b and returns an up part to assign to a
class _QueryMLHom : public StrongMLHom {
  int a,b;
public :
  _QueryMLHom (int aa, int bb) : a(aa),b(bb) {}

  HomHomMap phi (int var,int val) const {
    GHom homup = GHom::id;
    MLHom homdown = MLHom::id;
    if (var == b) {
      homdown = GHom (var,val);
      homup = AssignConst(a,val);
    } else {
      homdown = MLHom(var,val,this);
    }
    HomHomMap ret;
    ret.add(homup,homdown);
    return ret;
  }

  bool operator== (const StrongMLHom & s) const {
    _QueryMLHom * ps = (_QueryMLHom *)&s;
    return a == ps->a && b == ps->b;    
  }

  size_t hash() const {
    return 7489*(a^b);
  }


};

MLHom queryML (int a, int b) {
  return new _QueryMLHom(a,b);
}

// perform varl := varr independently of variable ordering.
class _AssignVar:public StrongHom {
  int varl;
  int varr;
public:
  _AssignVar(int left, int right) : varl(left), varr(right) {}
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   

  bool
  skip_variable(int var) const
  {
    return var != varr && var != varl ;
  }

  GHom phi(int vr, int vl) const {
    if (vr == varr) {
      // simple case, encounter varr before varl
      // Basic homomorphisms can handle this case.
      return GHom(vr,vl, AssignConst(varl,vl));
    } else {
      // must be varl given the skip variable constraint      
      // set stop level and propagate an MLHom
      return GHom(MLHom(vr,vl,queryML(varl,varr)));
    }
  }
  size_t hash() const {
    return 7717*varl^varr;
  }
  bool operator==(const StrongHom &s) const {
    _AssignVar* ps = (_AssignVar*)&s;
    return varl == ps->varl && varr == ps->varr;
  }
};

GHom AssignVar (int var, int val) {
  return new _AssignVar(var,val);
}


int main () {
  initName();

  DDD test1 = GDDD(A,1,GDDD(B,2,GDDD(C,3,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test2 = GDDD(A,2,GDDD(B,2,GDDD(C,3,GDDD(D,5,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test3 = GDDD(A,1,GDDD(B,2,GDDD(C,12,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,9)))))));
  DDD test4 = GDDD(A,1,GDDD(B,2,GDDD(C,12,GDDD(D,4,GDDD(E,5,GDDD(F,6,GDDD(G,7)))))));
  DDD test5 = test1 + test2 + test3 + test4;

  GHom be = AssignVar(B,E);
  GHom eb = AssignVar(E,B);
  GHom bg = AssignVar(B,G);


  cout << "Input :" << test5 << endl;
  cout << "b:=e :" << be(test5) << endl;
  cout << "e:=b :" << eb(test5) << endl;
  cout << "b:=g :" << bg(test5) << endl;

  return 0;
}
