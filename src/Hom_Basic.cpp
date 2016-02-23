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
  
	const GHom::range_t  get_range () const {
		GHom::range_t ret;
		ret.insert(var) ;
		return ret;
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
  
  GHom compose (const GHom & other) const {
    const _GHom * c = get_concret(other);
    if (typeid(*c) == typeid(_VarCompState)) {
      const _VarCompState * cc = (const _VarCompState *)c;
      if (cc->var == var) {
	const _VarCompState * cc1, * cc2;
	if (comp == EQ) {
	  cc1 = this;
	  cc2 = cc;
	} else {
	  cc1 = cc;
	  cc2 = this;
	}
	
	if (cc1->comp == EQ) {
	  if (cc2->comp == EQ) {
	    if (cc1->val == cc2->val) {
	      // [x=a] & [x=a] = [x=a]
	      return this;
	    } else {
	      // [x=a] & [x=b] = null if a <> b
	      return GDDD::null;
	    }
	  } else if (cc2->comp == LT) {
	    if (cc2->val > cc1->val) {
	      // [x=a] & [x<b] = [x=a] if a < b
	      return cc1;
	    } else {
	      // [x=a] & [x<b] = null if a >= b
	      return GDDD::null;
						}
	  }
	} else if (cc1->comp == LT) {
	  if (cc2->comp == LT) {
	    if (cc1->val < cc2->val) {
	      // [x<a] & [x<b] = [x<a] if a < b
	      return cc1;
	    } else {
	      // [x<a] & [x<b] = [x<a] if a >= b
	      return cc2;
	    }
	  }
	}
	// \todo other cases to be treated
	// std::cerr << "please improve composition of basic homs : no semantic composition of :" << GHom(this) << " and " << other << std::endl;
      }
    }
    return _GHom::compose(other);
  }
	 
  
  size_t hash() const {
    //return 8097*(var+2)^val * comp;
    return ddd::wang32_hash( 8097 * ((var << 16) | (val << 8)) ^ (comp));
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
  
	const GHom::range_t  get_range () const {
		GHom::range_t ret;
		ret.insert(var) ;
		return ret;
    }
	
  GDDD phiOne() const {
    return GDDD::one;
  }                   
  
  GHom phi(int vr, int vl) const {
    if (var == vr)
      return GHom(vr, val, GHom::id);
    return GHom(vr, vl, GHom(this)); 
  }

  GHom invert (const GDDD & potall) const { 
    std::set<GHom> sum;
    GDDD pot = computeDomain(var,potall);
    for (GDDD::const_iterator it = pot.begin() ; it != pot.end() ; ++it ) {
      sum.insert ( setVarConst(var,it->first) );
    }
    return GHom::add(sum) & varEqState (var,val);
  }

  
  size_t hash() const {
    return 6619*(ddd::int32_hash(var) ^ ddd::int32_hash(val));
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
    
	const GHom::range_t  get_range () const {
		GHom::range_t ret;
		ret.insert(target) ;
		return ret;
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
    return 9049*(ddd::int32_hash(target) ^ ddd::int32_hash(val));
//    return 9049* target^val;
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

static comparator invertComp(comparator comp) {
	switch (comp) {
    case EQ : case NEQ :
      return comp;
    case LT :
      return GT;
    case GT :
      return LT;
    case LEQ :
      return GEQ;
    case GEQ :
      return LEQ;
    default:
      assert(false);
      return EQ;
    }
}

class _VarCompVar : public StrongHom {	
	int var1;
	int var2;
	comparator c;
public:
	_VarCompVar(int v1, comparator c, int v2) : var1(v1), var2(v2), c(c) {}
	
	bool skip_variable(int vr) const {
		return vr != var1 && vr != var2;
	}
	
	const GHom::range_t  get_range () const {
		GHom::range_t ret;
		ret.insert(var1) ;
		ret.insert(var2) ;
		return ret;
    }
	
	GDDD phiOne() const {
		return GDDD::one;
	}                   
	
	GHom phi(int vr, int vl) const {
	  if (vr == var1) {
	    return GHom(vr,vl,varCompState(var2,invertComp(c),vl));
	  } else { // vr == var2
	    return GHom(vr,vl,varCompState(var1,c,vl));
	  }
	}
	
	GHom compose (const GHom & other) const {
		const _GHom * h = get_concret(other);
		if (typeid(*h) == typeid(_VarCompVar)) {
			const _VarCompVar * cc = (const _VarCompVar *)h;
			if ((var1 == cc->var1 && var2 == cc->var2) || (var2 == cc->var1 && var1 == cc->var2)) {
				if (c == EQ && cc->c != EQ) {
					// [x==y] & [x??y] = null
					return GDDD::null;
				} else if (c == EQ && cc->c == EQ) {
					// [x==y] & [x==y] = [x==y]
					return this;
				} else if (c == LT && cc->c == LT) {
					if (var1 != cc->var1) {
						// [x < y] & [x > y] = null
						return GDDD::null;
					} else {
						// [x < y] & [x < y] = [x < y]
						return this;
					}
				}
				/// \todo other cases to be treated
				std::cerr << "please improve composition of basic homs" << std::endl;
			}
		}
		
		return _GHom::compose(other);
	}
	
	size_t hash() const {
		return 6073*(var1+2)^var2 * c;
	}
	
	bool is_selector () const {
		return true;
	}
	
	// invert : already handled by default selector implem
	
	void print (std::ostream & os) const {
		os << "[ " << DDD::getvarName(var1) << to_string(c) << DDD::getvarName(var2) << " ]";
	}
	
	bool operator==(const StrongHom &s) const {
		_VarCompVar* ps = (_VarCompVar*)&s;
		return var1 == ps->var1 && var2 == ps->var2 && c == ps->c;
	}
    _GHom * clone () const {  return new _VarCompVar(*this); }
};

GHom varCompVar (int var, comparator c , int var2) {
  return _VarCompVar(var, c , var2);
}

GHom varEqVar (int var, int var2) {
	if (var == var2) {
		return GHom::id;
	}
	if (var < var2)
		return varCompVar(var,EQ,var2);
	else
		return varCompVar(var2,EQ,var);
}

GHom varNeqVar (int var, int var2) {
	if (var == var2) {
		return GHom(GDDD::null);
	}
	if (var < var2)
		return varCompVar (var,NEQ,var2);
	else
		return varCompVar (var2,NEQ,var);
}

GHom varGeqVar (int var, int var2) {
	if (var == var2) {
		return GHom::id;
	}
	return varCompVar (var,GEQ,var2);
}

GHom varGtVar (int var, int var2) {
	if (var == var2) {
		return GHom(GDDD::null);
	}
  return varCompVar (var,GT,var2);
}

GHom varLeqVar (int var, int var2) {
	if (var == var2) {
		return GHom::id;
	}
 return varCompVar (var,LEQ,var2);
}

GHom varLtVar (int var, int var2) {
	if (var == var2) {
		return GHom(GDDD::null);
	}
  return varCompVar (var,LT,var2);
}
