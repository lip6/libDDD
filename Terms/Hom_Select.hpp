#ifndef __HOM_SELECT_H_
#define __HOM_SELECT_H_

/** A selector homomorphism.  
 *  Bypass and restitute nodes until type_condition_ variable is met.
 *  Then keep only condition_ \inter arc value and apply next_ to son node.
 *  We expect : 
 * 1. type_condition_ variable exists on this set of path (or you get TOP);
 * 2. condition is a DataSet that should be compatible (to apply intersection) with the arc values of this variable
 * 3. next_ should have the same type of non-modifying selection behavior ; overall the operation behavior is to select paths from an SDD.
 * */
class _select_hom : public StrongShom {
  /// target variable
  int type_condition_;
  /// condition to intersect with
  const DataSet* condition_;
  /// hom to apply on successor
  GShom next_;

public:
  _select_hom(int type_condition,
	     const DataSet* condition,
	     const GShom& next)
    : type_condition_(type_condition),
      condition_(condition),
      next_(next)
  {}

  // should not reach terminal
  GSDD phiOne() const {
    return GSDD::top;
  }     
  
  GShom phi(int vr, const DataSet & vl) const {
//    cout << " running select on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;
    if (vr == type_condition_) {
      // we know there is only one level of depth, therefore DataSet concrete type is IntDataSet

      // dirty "delete" code due to use of DataSet interface instead of direct concrete IntDataSet/SDD manipulation
      DataSet * tofree =  vl.set_intersect(*condition_);
      GShom result = GShom( vr, *tofree, next_);
      delete tofree;
      return result;
    } else {
      // propagate
      return GShom(vr,vl,this);
    }
  }

  void mark() const {
    next_.mark();
  }  
  
  size_t hash() const {
    return ::__gnu_cxx::hash<int>()(type_condition_) ^  condition_->set_hash() ^ ::__gnu_cxx::hash<GShom>()(next_) ^ 12269; 
  }

  bool operator==(const StrongShom &s) const {
    const _select_hom * ps = (const _select_hom *)&s;
    return next_ ==  ps->next_
      && condition_ == ps ->condition_
      && type_condition_ == ps->type_condition_;
  }  
  
};

GShom select_hom(int type_condition,
	     const DataSet* condition,
		 const GShom& next = GShom::id) 
{
  return new _select_hom (type_condition,condition,next);
}


/** A selector homomorphism, with support for more generic condition.  
 *  Bypass and restitute nodes until type_condition_ variable is met.
 *  Then keep only condition_ ( arc value ) and apply next_ to son node.
 *  We expect : 
 * 1. type_condition_ variable exists on this set of path (or you get TOP);
 * 2. condition is SHom ; so that the variable target type_condition_ should have SDD referenced type.
 * 3. BOTH condition_ AND next_ should have the same type of non-modifying selection behavior ; overall the operation behavior is to select paths from an SDD.
 * */
class _select_deephom : public StrongShom {
  int type_condition_;
  GShom condition_;
  GShom next_;

public:
  _select_deephom(int type_condition,
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
//    cout << " running _select_deephom on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;
    if (vr == type_condition_) {
      SDD subresult = condition_((const SDD&) vl);
      return GShom(vr, subresult, next_);
    } else {
      return GShom(vr, vl, this);
    }
  }

  void mark() const {
    next_.mark();
    condition_.mark();
  }  
  
  size_t hash() const {
    return ::__gnu_cxx::hash<int>()(type_condition_) ^ ::__gnu_cxx::hash<GShom>()(condition_) ^ ::__gnu_cxx::hash<GShom>()(next_) ^ 12269; 
  }

  bool operator==(const StrongShom &s) const {
    const _select_deephom * ps = (const _select_deephom *)&s;
    return next_ ==  ps->next_
      && condition_ == ps ->condition_
      && type_condition_ == ps->type_condition_;
  }  
  
};

GShom  select_deephom(int type_condition,
		 const GShom& condition,
		 const GShom& next) 
{
  return new _select_deephom(type_condition,condition,next);
}


/** A helper homomorphism to change the depth of referenced values.
  * Descends the SDD looking for trigger_ variable, destroying input as it goes.
  * When trigger_ is reached return extractor(arc value).
  * Default extractor is id, just return the arc value.
  *  We expect : 
  * 1. trigger_ variable exists on this set of path (or you get TOP);
  * 2. trigger_ node has SDD as arc values type, since extractor is Shom. 
*/
class _extract_value : public StrongShom {
  int trigger_ ;
  GShom extractor_;
public :
  _extract_value (int trigger, const GShom & extractor) : trigger_(trigger),
								      extractor_(extractor) {}

  GSDD phiOne() const {
    return GSDD::top;
  }     
  
  // accept any NAT path with    0 +  X  \forall X
  GShom phi(int vr, const DataSet & vl) const {
//    cout << " running extractor on vr=" << vr << " vl= " ; vl.set_print(cout) ; cout << endl ;

    if (vr != trigger_) {
      // Don't test anything, propagate until right trigger is reached ...
      return this ;
    } else {
      // drop a level
      return SDD ( extractor_ ((SDD &) vl) );
    }
  }

  size_t hash() const {
    return 13913 ^ ::__gnu_cxx::hash<GShom> () (extractor_) ^ ::__gnu_cxx::hash<int> () (trigger_);
  }

  bool operator==(const StrongShom &s) const {
    const _extract_value * ps = (const _extract_value *)&s;
    return trigger_ == ps ->trigger_
      && extractor_ ==  ps->extractor_ ;
  }
  
};

GShom extract_value (int trigger, const GShom & extractor = GShom::id) {
  return new _extract_value (trigger,extractor);
}

#endif // __HOM_SELECT_H_
