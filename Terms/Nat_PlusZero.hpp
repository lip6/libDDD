#ifndef NAT_PLUS_ZERO__HH
#define NAT_PLUS_ZERO__HH

#include "Nat_Const.hpp"
#include "Hom_Select.hpp"

class _zero_plus_X : public StrongShom {

public :

  GSDD phiOne() const {
    return GSDD::one;
  }     
  
  // accept any NAT path with    0 +  X  \forall X
  GShom phi(int vr, const DataSet & vl) const {
    cout << " running zeroplusX on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;

    if (vr != RIGHT) {
      // Don't test anything, propagate until right is reached ...
      return this ;
    } else {
      // drop a level
      return (SDD &) vl ;
    }
  }

  size_t hash() const {
    return 13913;
  }

  bool operator==(const StrongShom &s) const {
    return true;
  }
  
};

SDD applyZeroPlusX (const SDD & d) {
  GShom select_zero_plus_test = select_hom(NAT, &natPlus, select_hom(LEFT, &SDDnatZero, GShom::id));
  GSDD d1 = select_zero_plus_test(d);
  return GShom(new _zero_plus_X()) (d1) 
    + (d-d1);
}

#endif // NAT_PLUS_ZERO__HH
