#include "SetVar.hh"

/********************************************************/
/* Define the strongHom _SetVar : var2 = var1           */
/* using 3 strongHoms : _SetCst, _SetVarUp, _SetVarDown */
/********************************************************/

class _SetCst:public StrongHom {
  int var, val;
public:
  _SetCst(int vr, int vl):var(vr),val(vl) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (var == vr)
      return GHom(vr,val);
    else 
      return GHom(vr,vl,GHom(this)); 
  }

  size_t hash() const {
    return var+val;
  }

  bool operator==(const StrongHom &s) const {
    _SetCst* ps = (_SetCst*)&s;
    return var == ps->var && val == ps->val;
  }

  _GHom * clone () const {  return new _SetCst(*this); }
};

// **********************************************
class _SetVarUp:public StrongHom {
  int var, val; 
public:
  _SetVarUp(int vr, int vl):var(vr),val(vl) {}

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    return GHom(vr,vl,GHom(var,val)); 
  }

  size_t hash() const {
    return var+val;
  }

  bool operator==(const StrongHom &s) const {
    _SetVarUp* ps = (_SetVarUp*)&s;
    return var == ps->var && val == ps->val;
  }

  _GHom * clone () const {  return new _SetVarUp(*this); }
};

// **********************************************
class _SetVarDown:public StrongHom {
  int var1, var2; // var1 <= var2
public:
  _SetVarDown(int v1, int v2):var1(v1),var2(v2) {}

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    if (var2 == vr)
      return GHom(var1,vl,GHom(vr,vl)); 
    else
      return GHom(_SetVarUp(vr,vl)) & GHom(this);
  }

  size_t hash() const {
    return var1+var2;
  }

  bool operator==(const StrongHom &s) const {
    _SetVarDown* ps = (_SetVarDown*)&s;
    return var1 == ps->var1 && var2 == ps->var2;
  }
  _GHom * clone () const {  return new _SetVarDown(*this); }
};

// **********************************************
class _SetVar:public StrongHom {
  int var1, var2; // var1 <= var2
public:
  _SetVar(int v1, int v2):var1(v1),var2(v2) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (var2 == vr)
      return GHom(vr,vl,_SetCst(var1,vl)); 
    if (var1 != vr)
      return GHom(vr,vl,GHom(this));
    return _SetVarDown(var1, var2);
  }

  size_t hash() const {
    return var1+var2;
  }

  bool operator==(const StrongHom &s) const {
    _SetVar* ps = (_SetVar*)&s;
    return var1 == ps->var1 && var2 == ps->var2;
  }
  _GHom * clone () const {  return new _SetVar(*this); }
};

// User function : Construct a Hom for a Strong Hom _SetVar
GHom setCst(int var,int val){return _SetCst(var,val);};
GHom setVar(int var1,int var2){return _SetVar(var1,var2);};
