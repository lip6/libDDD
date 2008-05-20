#include "MLHom.h"


// assign a constant to a value
class _AssignConst:public StrongHom {
  int var;
  int val;
public:
  _AssignConst(int vr, int vl) : var(vr), val(vl) {}
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   

  bool
  skip_variable(int var) const
  {
    return var != this->var;
  }

  GHom phi(int, int) const {
    return GHom(var, val);
  }
  size_t hash() const {
    return 6619*var^val;
  }
  bool operator==(const StrongHom &s) const {
    _AssignConst* ps = (_AssignConst*)&s;
    return var == ps->var && val == ps->val;
  }
  _GHom * clone () const {  return new _AssignConst(*this); }
};

GHom AssignConst (int var, int val) {
  return new _AssignConst(var,val);
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

  _MLHom * clone () const {  return new _QueryMLHom(*this); }

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
  _GHom * clone () const {  return new _AssignVar(*this); }
};

GHom AssignVar (int var, int val) {
  return new _AssignVar(var,val);
}


typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}

#include <iostream>
using namespace std;

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
