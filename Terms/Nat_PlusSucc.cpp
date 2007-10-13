#include "Nat_PlusSucc.hpp"
#include "Nat_Const.hpp"
#include "Hom_Select.hpp"

class _succ_plus_X_stored : public StrongShom {

  GSDD X;
public :
  _succ_plus_X_stored (const GSDD & xx) : X(xx) {};

  
  GSDD phiOne() const {
    return GSDD::top;
  }     
 
  // rewriting  X + succ(Y) to succ (X + Y)
  GShom phi(int vr, const DataSet & vl) const {
    if (vr == RIGHT ) {
      // so vl is a SDD containing Y
      // produce output
      return SDDnatSucc 
	^ GSDD (SUCCARG, 
		SDDnatPlus 
		^ SDD (LEFT,X)  
		^ SDD (RIGHT, 
		       extract_value(SUCCARG) ((const SDD&)vl))
		);
    } else {
      // should not happen
	return GSDD::top;
    }
  }

  bool operator==(const StrongShom &s) const {
    _succ_plus_X_stored * ps = (_succ_plus_X_stored *)&s;
    return X == ps->X;
  }

  void mark() const {
    X.mark();
  }  

  size_t hash() const {
    return  2357 ^ __gnu_cxx::hash<GSDD>()(X)  ;
  }

};


class _succ_plus_XY : public StrongShom {

public :

  GSDD phiOne() const {
    return GSDD::one;
  }     
  
  // rewriting  X + succ(Y) to succ (X + Y)
  GShom phi(int vr, const DataSet & vl) const {
//    cout << " running succ XY on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;
    if (vr != LEFT) {
      // Don't test anything, propagate until left is reached ...
      return this ;
    } else {
      // store X value read and continue
      return new _succ_plus_X_stored ((const SDD &) vl) ;
    }
  }

  size_t hash() const {
    return 14387;
  }

  bool operator==(const StrongShom &s) const {
    return true;
  }
  
};

SDD applySuccPlusXY (const SDD & d) {
  GShom select_succ = select_hom(NAT, &natSucc, GShom::id);
  GShom select_plus_x_succ_y = select_hom(NAT, &natPlus, select_deephom(RIGHT, select_succ, GShom::id));
  GSDD d1 = select_plus_x_succ_y(d);
  return GShom(new _succ_plus_XY()) (d1) 
    + (d-d1);
}
