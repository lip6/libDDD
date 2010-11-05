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
  
  bool is_selector () const {
    return true;
  }

  // invert : already handled by default selector implem

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

GHom varLtState (int var, int val) {
  return varCompState (var,LT,val);
}

GHom varGeqState (int var, int val) {
 return varCompState (var,GEQ,val);
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

  GHom invert (const GDDD & pot) const { 
    std::set<GHom> sum;
    for (GDDD::const_iterator it = pot.begin() ; it != pot.end() ; ++it ) {
      sum.insert ( setVarConst(var,it->first) );
    }
    return GHom::add(sum) & varEqState (var,val);
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

class _incVar :public StrongHom {
  int target;
  int val;
public:
  _incVar (int var, int val) : target(var), val(val) {}
  
  bool
  skip_variable(int var) const
  {
    return target != var;
  }
    
  GHom invert (const GDDD & pot) const { 
    return incVar(target, -val);
   }


  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (target == vr) {
      // Just a trick to detect if we are going unbounded ...
      //	    if (vl > 1) std::cerr << "reached value " << vl << " for place " << DDD::getvarName(vr) << std::endl; 
      return GHom(vr, vl + val );
    } else {
      // Should not happen with skip_variable
      return GHom(vr,vl,this);
    }
  }
  
  size_t hash() const {
    return 9049* target^val;
  }
  
  
  bool operator==(const StrongHom &s) const {
    _incVar* ps = (_incVar*)&s;
    return target == ps->target && val == ps->val;
  }

  _GHom * clone () const {  return new _incVar(*this); }

  void print (std::ostream & os) const {
    os << "[ " << DDD::getvarName(target) << " += " << val << " ]";
  }

};



GHom incVar (int var, int val) {
  return _incVar(var,val);
}
