#include "SwapMLHom.hh"
#include "MLHom.h"
#include "Hom_Basic.hh"

class _SwapQuery : public StrongMLHom {
  int var;
  int val;
  int vset;
public:
  _SwapQuery(int vr, int vl, int vrs): var(vr), val(vl), vset(vrs) {}
  
  // actually not useful currently
  bool skip_variable(int v) const {
    return v != var;
  }
    
  HomNodeMap phiOne() const {
    HomNodeMap res;
    std::cerr << "MLHom should not reach this point" << std::endl;
    assert(false);
    return res;
  }
    
  HomHomMap phi(int vr, int vl) const {
    // the skip_variable mechanism is not implemented yet, so this is mandatory
    if (vr == var) {
      GHom homup = GHom(vset, vl);
      GHom homdown = GHom(vr, val, GHom::id);

      HomHomMap ret;
      ret.add(homup, homdown);
      return ret;
    } else {
      HomHomMap ret;
      ret.add(GHom::id, MLHom(vr, vl, *this));
      return ret;
    }
  }
    
  size_t hash() const {
    return d3::util::hash<int>()(var) * d3::util::hash<int>()(val) ^ d3::util::hash<int>()(vset);
  }
    
  bool operator==(const StrongMLHom &s) const {
    const _SwapQuery * ss = (const _SwapQuery *)&s;
    return var == ss->var && val == ss->val && vset == ss->vset;
  }
    
  _MLHom * clone() const {
    return new _SwapQuery(*this);
  }
};

/**
 get down the DDD until a variable is found, then uses a MLHom
 */
class _Swap : public StrongHom {
	int var1;
  int var2;
public:
	_Swap(int v1, int v2) : var1(v1), var2(v2) {}
	
	// affects only the swapped variables
	bool skip_variable(int vr) const {
		return (vr != var1) && (vr != var2);
	}
	
	GDDD phiOne() const {
		return GDDD::one;
	}                   
	
	GHom phi(int vr, int vl) const {
    if (vr == var1) {
      return MLHom(_SwapQuery(var2, vl, var1));
    } else if (vr == var2) {
      return MLHom(_SwapQuery(var1, vl, var2));
    } else { // should not happen, due to skip
      assert(false);
      return GHom::id;
    }
	}
	
	size_t hash() const {
		return 18043*(var1^(var2+1));
	}
	
	void print (std::ostream & os) const {
		os << "[ Swap(" << DDD::getvarName(var1) << "," << DDD::getvarName(var2) << ") ]";
	}
	
	bool operator==(const StrongHom &s) const {
		_Swap* ps = (_Swap*)&s;
		return var1 == ps->var1 && var2 == ps->var2;
	}
    _GHom * clone () const {  return new _Swap(*this); }
};

GHom Swap(int i,int j) {
  if (i == j) {
    return GHom::id;
  }
  return _Swap(std::min(i, j), std::max(i, j));
}
