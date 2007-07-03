#ifndef NAT_PLUS_SUCC__HH
#define NAT_PLUS_SUCC__HH

#include "Nat_Const.hpp"


// rewriting rule : X + succ(Y) -> succ( X+Y)


class select_deephom : public StrongShom {
  int type_condition_;
  GShom condition_;
  GShom next_;

public:
  select_deephom(int type_condition,
		 const GShom& condition,
		 const GShom& next)
    : type_condition_(type_condition),
      condition_(condition),
      next_(next)
  {}

  GSDD phiOne() const {
    return GSDD::top;
  }     
  
  GShom phi(int vr, const DataSet & vl) const {
    cout << " running select_deephom on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;
    if (vr == type_condition_) {
      SDD subresult = condition_((const SDD&) vl);
      return GShom(vr, subresult, next_);
    } else {
      return GShom(vr, vl, this);
    }
  }

  void mark() const {
    next_.mark();
  }  
  
  size_t hash() const {
    return ::__gnu_cxx::hash<int>()(type_condition_) ^ ::__gnu_cxx::hash<GShom>()(condition_) ^ ::__gnu_cxx::hash<GShom>()(next_) ^ 12269; 
  }

  bool operator==(const StrongShom &s) const {
    const select_deephom * ps = (const select_deephom *)&s;
    return next_ ==  ps->next_
      && condition_ == ps ->condition_
      && type_condition_ == ps->type_condition_;
  }  
  
};



class _select_succ : public StrongShom {
public :
  GSDD phiOne() const {
    return GSDD::one;
  }     
  
  // accept any NAT path with    X + succ( Y )  \forall X,Y
  GShom phi(int vr, const DataSet & vl) const {
    cout << " running select succ on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;
    
    if ( vr == NAT ) {
      // we know there is only one level of depth, therefore DataSet concrete type is IntDataSet

      // looks good : looking for  "succ" paths
      // paths with anything except "succ" are killed
      DataSet * tofree =  vl.set_intersect(natSucc);
      if (! tofree->empty() ) {
	delete tofree;
	return SDDnatSucc ; // implicit id
      } else {
	// kill path
	delete tofree;
	return GSDD::null ;
      }

    } else {
      assert (false);
    }

  }

  size_t hash() const {
    return 14347;
  }

  bool operator==(const StrongShom &s) const {
    return true;
  }


};


class _succ_plus_test : public StrongShom {

public :

  GSDD phiOne() const {
    return GSDD::one;
  }     
  
  // accept any NAT path with    X + succ( Y )  \forall X,Y
  GShom phi(int vr, const DataSet & vl) const {
    cout << " running succplus on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;


    if (vr == NAT) {
      // we know there is only one level of depth, therefore DataSet concrete type is IntDataSet

      // looks good : looking for  "+" paths
      // paths with anything except "PLUS" should be left alone
      DataSet * tofree =  vl.set_minus(natPlus);
      if ( tofree->empty() ) {
	delete tofree;
	return SDDnatPlus ^ this ;
      } else {
	// kill path
	delete tofree;
	return GSDD::null ;
      }
    } else if (vr == LEFT) {
      // we know argument should be a NAT, therefore DataSet concrete type is SDD
      
      // store and continue
      return GShom (vr, vl , this) ;
    } else {
      // vr == RIGHT
      return GShom ( vr , SDD(GShom(new _select_succ()) ( (const SDD &) vl ) ));
    }
  }

  size_t hash() const {
    return 13127;
  }

  bool operator==(const StrongShom &s) const {
    return true;
  }
  
};

class _succ_plus_X_stored : public StrongShom {

  GSDD X;
public :
  _succ_plus_X_stored (const GSDD & xx) : X(xx) {};

  
  GSDD phiOne() const {
    return GSDD::one;
  }     
 
  // rewriting  X + succ(Y) to succ (X + Y)
  GShom phi(int vr, const DataSet & vl) const {
    if (vr == SUCCARG ) {
      // so vl is a SDD containing Y
      // produce output
      return SDDnatSucc ^ GSDD (SUCCARG, SDD (SDDnatPlus ^ SDD(LEFT,SDD(X)) ^SDD (RIGHT, vl))) ;
    } else {
      // skip a bit
      return this;
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
    cout << " running succ XY on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;
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
  GShom select_succ = new select_hom(NAT, &natSucc, GShom::id);
  GShom select_plus_x_succ_y = new select_hom(NAT, &natPlus, new select_deephom(RIGHT, select_succ, GShom::id));
  //  GSDD d1 = GShom(new _succ_plus_test()) (d);
  GSDD d1 = select_plus_x_succ_y(d);
  return GShom(new _succ_plus_XY()) (d1) 
    + (d-d1);
}

#endif // NAT_PLUS_SUCC__HH
