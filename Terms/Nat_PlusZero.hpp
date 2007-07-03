#ifndef NAT_PLUS_ZERO__HH
#define NAT_PLUS_ZERO__HH

#include "Nat_Const.hpp"

class select_hom : public StrongShom {
  int type_condition_;
  const DataSet* condition_;
  GShom next_;

public:
  select_hom(int type_condition,
	     const DataSet* condition,
	     const GShom& next)
    : type_condition_(type_condition),
      condition_(condition),
      next_(next)
  {}

  GSDD phiOne() const {
    return GSDD::one;
  }     
  
  GShom phi(int vr, const DataSet & vl) const {
    cout << " running zeroplus on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;
    if (vr == type_condition_) {
      // we know there is only one level of depth, therefore DataSet concrete type is IntDataSet

      // looks good : looking for  "+" paths
      // paths with anything except "PLUS" should be left alone
      // dirty "delete" code due to use of DataSet interface instead of direct IntDataSet manipulation
      DataSet * tofree =  vl.set_intersect(*condition_);
      Shom result = GShom( vr, *tofree, next_);
      delete tofree;
      return result;
    } else {
      // should not reach this point ??
      assert(false);
      return SDD::null;
    }
  }

  void mark() const {
    next_.mark();
  }  
  
  size_t hash() const {
    return ::__gnu_cxx::hash<int>()(type_condition_) ^  condition_->set_hash() ^ ::__gnu_cxx::hash<GShom>()(next_) ^ 12269; 
  }

  bool operator==(const StrongShom &s) const {
    const select_hom * ps = (const select_hom *)&s;
    return next_ ==  ps->next_
      && condition_ == ps ->condition_
      && type_condition_ == ps->type_condition_;
  }  
  
};

class _zero_plus_test : public StrongShom {
  GShom h ;
public :
  _zero_plus_test (const GShom hh) : h(hh) {};

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
      // dirty "delete" code due to use of DataSet interface instead of direct IntDataSet manipulation
      DataSet * tofree =  vl.set_minus(natPlus);
      if ( tofree->empty() ) {
	delete tofree;
	return SDDnatPlus ^ h ;
      } else {
	// kill path
	delete tofree;
	return GSDD::null ;
      }
    } else if (vr == LEFT) {
      // we know argument should be a NAT, therefore DataSet concrete type is SDD
      // look for paths with a zero
      return GShom( vr , SDD (((const SDD &) vl) * SDDnatZero));

    } else {
      // should not reach this point ??
      assert ( false );
    }
  }

  void mark() const {
    h.mark();
  }  
  
  size_t hash() const {
    return  ::__gnu_cxx::hash<GShom>()(h) ^ 12269; 
  }

  bool operator==(const StrongShom &s) const {
    _zero_plus_test * ps = (_zero_plus_test *)&s;
    return h ==  ps->h;
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
  GShom select_zero_plus_test = new select_hom(NAT, &natPlus, new select_hom(LEFT, &SDDnatZero, GShom::id));
  //  GSDD d1 = GShom(new _zero_plus_test()) (d);
  GSDD d1 = select_zero_plus_test(d);
  return GShom(new _zero_plus_X()) (d1) 
    + (d-d1);
}

#endif // NAT_PLUS_ZERO__HH
