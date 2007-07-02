#ifndef NAT_PLUS_ZERO__HH
#define NAT_PLUS_ZERO__HH

#include "Nat_Const.hpp"

class _zero_plus_test : public StrongShom {

public :

  GSDD phiOne() const {
    return GSDD::one;
  }     
  
  // accept any NAT path with    0 +  X  \forall X
  GShom phi(int vr, const DataSet & vl) const {
    cout << " running zeroplus on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;


    if (vr == NAT) {
      // we know there is only one level of depth, therefore DataSet concrete type is IntDataSet

      // looks good : looking for  "+" paths
      // paths with anything except "PLUS" should be left alone
      DataSet * tofree =  vl.set_minus(natPlus);
      if ( tofree->empty() ) {
	delete tofree;
	return GShom (vr, natPlus , this) ;
      } else {
	// kill path
	delete tofree;
	return GSDD::null ;
      }
    } else if (vr == LEFT) {
      // we know argument should be a NAT, therefore DataSet concrete type is SDD
      
      // look for paths with a zero
      DataSet * tofree =  vl.set_intersect(SDDnatZero);
      if (! tofree->empty() ) {
	// a win ; detected 0 + X
	delete tofree;
	return GShom (vr, SDDnatZero ) ;
      } else {
	// kill path
	delete tofree;
	return GSDD::null ;
      }
    } else {
      // should not reach this point ??
      assert ( false );
    }
  }

  size_t hash() const {
    return 12101;
  }

  bool operator==(const StrongShom &s) const {
    return true;
  }
  
};


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


#endif // NAT_PLUS_ZERO__HH
