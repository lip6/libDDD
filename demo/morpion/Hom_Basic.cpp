#include "Hom_Basic.hh"


typedef enum comparator { EQ, NEQ, LT, GT, LEQ, GEQ} comparator;

std::string to_string (comparator comp) {
  switch (comp) {
    case EQ : 
      return "==";
    case NEQ :
      return "!=";
    case LT :
      return "<";
    case GT :
      return ">";
    case LEQ :
      return "<=";
    case GEQ :
      return ">=";
  default :
    return "??";
    }
}



class _VarCompState:public StrongHom {

  int var;
  int val;
  comparator comp;
public:
  _VarCompState(int vr, comparator c, int vl) : var(vr), val(vl), comp(c) {}
  
  
  bool is_selector () const {
    return true;
  }


  bool
  skip_variable(int vr) const
  {
    return vr != var;
  }
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   
  
  GHom phi(int vr, int vl) const {
    bool sel = false;
    switch (comp) {
    case EQ : 
      sel = (vl == val);
      break;
    case NEQ :
      sel = (vl != val);
      break ;
    case LT :
      sel = (vl < val);
      break;
    case GT :
      sel = (vl > val);
      break ;
    case LEQ :
      sel = (vl <= val);
      break;
    case GEQ :
      sel = (vl >= val);
      break;
    }
    if (sel)
      return GHom(vr, vl, GHom::id);
    else
      return GHom(GDDD::null);
  }
  
  size_t hash() const {
    return 8097*(var+2)^val * comp;
  }
  
  void print (std::ostream & os) const {
    os << "[ " << DDD::getvarName(var) << " " << to_string(comp) << " " << val << " ]";
  }

  bool operator==(const StrongHom &s) const {
    _VarCompState* ps = (_VarCompState*)&s;
    return comp == ps->comp && var == ps->var && val == ps->val;
  }
    _GHom * clone () const {  return new _VarCompState(*this); }
};

GHom varCompState (int var, comparator c , int val) {
  return _VarCompState(var, c , val);
}

GHom varEqState (int var, int val) {
  return varCompState(var,EQ,val);
}

GHom varNeqState (int var, int val) {
  return varCompState (var,NEQ,val);
}

GHom varGtState (int var, int val) {
  return varCompState (var,GT,val);
}

GHom varLeqState (int var, int val) {
 return varCompState (var,LEQ,val);
}


class _setVarConst:public StrongHom {

  int var;
  int val;
public:
  _setVarConst(int vr, int vl) : var(vr), val(vl) {}
  
  bool
  skip_variable(int vr) const
  {
    return vr != var;
  }
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   
  
  GHom phi(int vr, int vl) const {
    if (var == vr)
      return GHom(vr, val, GHom::id);
    return GHom(vr, vl, GHom(this)); 
  }
  
  size_t hash() const {
    return 6619*(var+13)^val;
  }
  
  void print (std::ostream & os) const {
    os << "[ " << DDD::getvarName(var) << " = "  << val << " ]";
  }

  bool operator==(const StrongHom &s) const {
    _setVarConst* ps = (_setVarConst*)&s;
    return var == ps->var && val == ps->val;
  }
    _GHom * clone () const {  return new _setVarConst(*this); }
};

GHom setVarConst (int var, int val) {
  return _setVarConst(var,val);
}
