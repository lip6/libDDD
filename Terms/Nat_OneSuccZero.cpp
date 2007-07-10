#include "Nat_OneSuccZero.hpp"
#include "Nat_Const.hpp"
#include "Hom_Select.hpp"


class _one_to_succ_0 : public StrongShom {

public :

  GSDD phiOne() const {
    return GSDD::one;
  }     
  
  // accept any NAT path 1
  GShom phi(int vr, const DataSet & vl) const {
//    cout << " running one_to_succ0 on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;
    assert (vr == NAT) ;
    
    // we know vl == {1} rewrite to succ(0)
    return  SDDnatSucc ^ GSDD ( SUCCARG, SDDnatZero);

  }

  size_t hash() const {
    return 12289 ;
  }

  bool operator==(const StrongShom &s) const {
    return true;
  }
  
};

SDD applyOneToSucc0 (const SDD & d) {
  GShom select_one_test = select_hom(NAT, &natOne);
  GSDD d1 = select_one_test(d);
  return GShom(new _one_to_succ_0()) (d1) 
    + (d-d1);
}
