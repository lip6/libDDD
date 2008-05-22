#include "PermuteVar.hh"

/***********************************************************/
/* Define the strongHom Permute : var1,var2 = var2,var1    */
/* using 3 strongHoms : PermuteUp, PermuteDown, PermuteFin */
/***********************************************************/

// **********************************************
class PermuteUp:public StrongHom {
  int var, val; // var1 <= var2
public:
  PermuteUp(int vr, int vl):var(vr),val(vl) {}

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    return GHom(vr,vl,GHom(var,val)); 
  }

  size_t hash() const {
    return var+val+400;
  }

  bool operator==(const StrongHom &s) const {
    PermuteUp* ps = (PermuteUp*)&s;
    return (var == ps->var && val == ps->val);
  }
  _GHom * clone () const {  return new PermuteUp(*this); }
};

// **********************************************
class PermuteDown:public StrongHom {
  int var, val; // var1 <= var2
public:
  PermuteDown(int vr, int vl):var(vr),val(vl) {}

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    if (var == vr)
      return GHom(0,vl,GHom(vr,val)); 
    if (var != vr)
      return GHom(PermuteUp(vr,vl)) & GHom(this);
    return GHom::id;
  }
 
  size_t hash() const {
    return var+val+300;
  }

  bool operator==(const StrongHom &s) const {
    PermuteDown* ps = (PermuteDown*)&s;
    return var == ps->var && val == ps->val;
  }
  _GHom * clone () const {  return new PermuteDown(*this); }
};


// **********************************************
class PermuteFin:public StrongHom {
  int var; // var1 <=> var2
public:
  PermuteFin(int vr):var(vr) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int, int vl) const {
    return GHom(var,vl);
  }

  size_t hash() const {
    return var+200;
  }

  bool operator==(const StrongHom &s) const {
    PermuteFin* ps = (PermuteFin*)&s;
    return var == ps->var;
  }
  _GHom * clone () const {  return new PermuteFin(*this); }
};

// **********************************************
class Permute:public StrongHom {
  int var1, var2; // var1 <=> var2
public:
  Permute(int v1, int v2):var1(v1),var2(v2) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (var1 == vr)
      return GHom(PermuteFin(var1)) & GHom(PermuteDown(var2,vl)); 
    if (var2 == vr)
      return GHom(PermuteFin(var2)) & GHom(PermuteDown(var1,vl)); 
    return GHom(vr,vl,GHom(this));
  }

  size_t hash() const {
    return var1+var2+100;
  }

  bool operator==(const StrongHom &s) const {
    Permute* ps = (Permute*)&s;
    return var1 == ps->var1 && var2 == ps->var2;
  }
  _GHom * clone () const {  return new Permute(*this); }
};

// User function : Construct a Hom for a Strong Hom _SetVar
GHom permute(int var1,int var2){return Permute(var1,var2);};
