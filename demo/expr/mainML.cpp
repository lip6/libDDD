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

  GHom phi(int vr, int vl) const {
    return GHom(var, val);
  }
  size_t hash() const {
    return 6619*var^val;
  }
  bool operator==(const StrongHom &s) const {
    _AssignConst* ps = (_AssignConst*)&s;
    return var == ps->var && val == ps->val;
  }
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

  bool operator== (const StrongMLHom & s) {
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
      return GHom( queryML(varl,varr));
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
