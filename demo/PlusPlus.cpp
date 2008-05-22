#include "PlusPlus.hh"
#include "hashfunc.hh"

// Increment the first value of var
class _plusplus:public StrongHom {
  int var;
public:
  _plusplus(int vr):var(vr) {};

  GDDD phiOne() const {
    return GDDD::one;
  }

  GHom phi(int vr, int vl) const {
    if (vr == var)
      return GHom(vr,vl+1);
    else
      return GHom(vr,vl,GHom(this));
  }

  size_t hash() const {
    return ddd::wang32_hash(var);
  }

  bool operator==(const StrongHom &s) const {
    _plusplus* ps = (_plusplus*)&s;
    return var == ps->var;
  }

  _GHom * clone () const {  return new _plusplus(*this); }
};

GHom plusplus(int vr){return _plusplus(vr);};


// Increment the first value of var
class _plusplusAll:public StrongHom {
  int var;
public:
  _plusplusAll(int vr):var(vr) {};

  GDDD phiOne() const {
    return GDDD::one;
  }

  GHom phi(int vr, int vl) const {
    if (vr == var)
      return GHom(vr,vl+1,GHom(this));
    else
      return GHom(vr,vl,GHom(this));
  }

  size_t hash() const {
    return ddd::wang32_hash(var);
  }

  bool operator==(const StrongHom &s) const {
    _plusplusAll* ps = (_plusplusAll*)&s;
    return var == ps->var;
  }

  _GHom * clone () const {  return new _plusplusAll(*this); }
};

GHom plusplusAll(int vr){return _plusplusAll(vr);};

/// increment the value of the next variable
class _plusplusFirst:public StrongHom {
public:
  _plusplusFirst() {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    return GHom(vr,vl+1);
  }

  size_t hash() const {
    return 23;
  }

  bool operator==(const StrongHom&) const {
    return true;
  }
  _GHom * clone () const {  return new _plusplusFirst(*this); }
};

/// User function : Construct a Hom for a Strong Hom _plusplus
GHom plusplusFirst(){return _plusplusFirst();};
