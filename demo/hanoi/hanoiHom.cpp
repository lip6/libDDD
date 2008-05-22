/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2008 Yann Thierry-Mieg                            */
/*     						                            */
/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU Lesser General Public License as       */
/*     published by the Free Software Foundation; either version 3 of the   */
/*     License, or (at your option) any later version.                      */
/*     This program is distributed in the hope that it will be useful,      */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*     GNU LEsserGeneral Public License for more details.                   */
/*     						                            */
/* You should have received a copy of the GNU Lesser General Public License */
/*     along with this program; if not, write to the Free Software          */
/*Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*     						                            */
/****************************************************************************/


#include "hanoiHom.hh"
#include "IntDataSet.h"
#include <vector>

// Global constants.
// we use one DDD variable per ring, ring 0 is the topmost, 
// and is stored at the bottom of the DDD
int NB_RINGS = 5;
// Each variable domain is {0,1,2} expressing the pole the variable is on
int NB_POLES = 3; 


void initName() 
{
  char buff [12];
  for (int i=0; i< NB_RINGS; i++) {
    sprintf(buff,"ring %d",i);
    DDD::varName(i,buff);
  }
}
// int -> string
std::string toString (int i) {
  char buff [16];
  sprintf (buff,"%d",i);
  return buff;
}
// float -> string
std::string toString (double i) {
  char buff [16];
  sprintf (buff,"%lf",i);
  return buff;
}


// Removes any path such that one of the variables takes value i or value j
// Not test on variable number means this operation should be used from "mid-height"
class _no_ring_above 
    : 
    public StrongHom
    {
        // the 2 poles that have to be clear
        int i_,j_;
        public :
        _no_ring_above (int i, int j) { 
            /// force to have ii < jj since the operation is commutable,  
            // _no_ring_above(i,j) = _no_ring_above(j,i)
            // hence we have a canonical form
            if (i < j) 
            {
                i_ = i; 
                j_ = j; 
            }
            else
            {
                i_ = j ;
                j_ = i ;
            }
        }
        
        GDDD phiOne() const {
            return GDDD::one;
        }     
        
        // reject any path with ANY ring that is on pole i or pole j
        GHom 
        phi(int vr, int vl) const {
            if ( vl == i_ || vl == j_ )
                // cut this branch and exploration
                return GDDD::null;
            else
                // propagate this test
                return GHom(vr,vl,GHom(this));
        }
        
        size_t hash() const {
            return (i_ &  j_<<16)  *  9749;
        }
        
        bool operator==(const StrongHom &s) const {
            _no_ring_above* ps = (_no_ring_above*)&s;
            return i_ == ps->i_ && j_ == ps->j_ ;
        }
  _GHom * clone () const {  return new _no_ring_above(*this); }        
    };


class _swap_pole : public StrongHom {
    // ring we are trying to move
	int ring_;
    // ori pole number
	int ori_;
    // dest pole number
	int dest_;
    
	public :
	_swap_pole (int ring, int ori, int dest )
    : 
    ring_(ring),
    ori_(ori),
    dest_(dest)
    {
    }
    
	GDDD
    phiOne() const
    {
		return GDDD::one;
	}                   
    
	GHom 
    phi(int vr, int vl) const
    {
		if (vr != ring_ ) 
        {
            // target ring not reached yet : propagate
			return GHom(vr,vl,this);
		} 
        else 
        {
            // ring reached : shift the pole if ori == vl
			if (ori_ != vl) 
            {
                // precondition not met for this event : we try to move ring from ori_ to dest_ but ring is NOT on ori_ !
				return GDDD::null;
			} 
            else 
            {
                // update ring position and test no ring above
                // no_ring_above propagates on the bottom of the DDD ; it returns 0 if preconditions are not met 
                // or a DDD with only paths where the move is legal
	      return GHom (ring_ , dest_) & GHom(_no_ring_above(ori_,dest_));
                // another way of writing it is 
                // return GHom (ring_ , dest_, _no_ring_above(ori_,dest_) );
			}
		}
	}
    
    bool
    skip_variable(int v) const
    {
        return  v != ring_;
    }
    
	size_t
    hash() const
    {
        // a hash function, probably too complex but will work well
        // be careful of multplying by 0 valued attributes...
		return ((6961*(ring_+1)+ 3) * (ori_+7877 )) ^ (dest_-1) ;
	}
    
	bool
    operator==(const StrongHom &s) const
    {
		_swap_pole* ps = (_swap_pole*)&s;
		return ring_ == ps->ring_ && ori_ == ps->ori_ && dest_ == ps->dest_ ;
	}
  _GHom * clone () const {  return new _swap_pole(*this); }
    
};

// to be more pleasant for users  
GHom swap_pole ( int ring, int ori, int dest ) {
	return _swap_pole (ring, ori,dest);
}

// move a ring to all possible other poles, based on current variable value.
class _move_ring : public StrongHom {
  // ring we are trying to move
  int ring_;
  
public :

    _move_ring (int ring)
    	: 
    ring_(ring) 
    {}
  
    bool
    skip_variable(int v) const
    {
      return   v != ring_ ;
    }
    
      GDDD phiOne() const 
    {
        return GDDD::one;
    }              
  
    GHom phi(int vr, int vl) const 
    {
        if (vr != ring_ ) 
        {
            // target ring not reached yet : propagate
            return GHom(vr,vl,this) ;
        }
        else 
        {
            // ring reached 
            // try to move to all new positions
            GHom res = GDDD::null;
            for (int i=0 ; i <NB_POLES ; i++)
            {
                // test all possible moves from current position = vl
                if (i != vl) {
                    // update ring position and test no ring above
                    // no_ring_above propagates on the bottom of the DDD ; it returns 0 if preconditions are not met 
                    // or a DDD with only paths where the move is legal
                    res = res + ( GHom (ring_ , i) &  _no_ring_above(i , vl) );
                }
            }
            return res ;
        }
    }
  
  size_t hash() const {
    // a hash function (avoid hash value 0)
    return 6961*(ring_+1);
  }
  
  bool operator==(const StrongHom &s) const {
    _move_ring* ps = (_move_ring*)&s;
    return ring_ == ps->ring_;
  }
  _GHom * clone () const {  return new _move_ring(*this); }
  
};

// to be more pleasant for users  
GHom move_ring ( int ring ) {
  return _move_ring (ring);
}



class _move_ring_id : public StrongHom {
  // ring we are trying to move
  int ring_;
  
public :
  _move_ring_id (int ring): ring_(ring) {};
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   
  
    bool
    skip_variable(int v) const
    {
        if(  v != ring_ )
        {
            return true;
        }
        return false;
    }
    
  
  GHom phi(int vr, int vl) const {
    if (vr != ring_ ) {
      // target ring not reached yet : propagate
      return GHom(vr,vl,this) ;
    } else {
      // ring reached 
      // try to move to all new positions
      // Initialize res with Id
      // THIS IS THE ONLY LINE THAT DIFFERS FROM move_ring
      GHom res = GHom(vr,vl) ;
      for (int i=0 ; i <NB_POLES ; i++) {
	// test all possible moves from current position = vl
	if (i != vl) {
	  // update ring position and test no ring above
	  // no_ring_above propagates on the bottom of the DDD ; it returns 0 if preconditions are not met 
	  // or a DDD with only paths where the move is legal
	  res = res + ( GHom (ring_ , i) & _no_ring_above(i , vl) );
	}
      }
      return res ;
    }
  }
  
  size_t hash() const {
    // a hash function (avoid hash value 0)
    return 6961*(ring_+1);
  }
  
  bool operator==(const StrongHom &s) const {
    _move_ring_id* ps = (_move_ring_id*)&s;
    return ring_ == ps->ring_;
  }
  _GHom * clone () const {  return new _move_ring_id(*this); }
  
};

// to be more pleasant for users  
GHom move_ring_id ( int ring ) {
  return _move_ring_id (ring);
}



// This version propagates a fixpoint
class _move_ring_sat : public StrongHom {
    // ring we are trying to move
    int ring_;
    
    public :
    _move_ring_sat (int ring): ring_(ring) {};
    
    GDDD phiOne() const {
        return GDDD::one;
    }                   
    
    bool
    skip_variable(int v) const
    {
        return v != ring_;
    }
    
    
    GHom phi(int vr, int vl) const {
        if (vr != ring_ ) {
            // target ring not reached yet : propagate
            return GHom(vr,vl,this) ;
        } else {
            // ring reached 
            // try to move to all new positions
            // Initialize res with Id
            GHom res = GHom(vr,vl) ;
            for (int i=0 ; i <NB_POLES ; i++) {
                // test all possible moves from current position = vl
                if (i != vl) {
                    // update ring position and test no ring above
                    // no_ring_above propagates on the bottom of the DDD ; it returns 0 if preconditions are not met 
                    // or a DDD with only paths where the move is legal
                    res = (res + ( GHom (ring_ , i) & _no_ring_above(i , vl) )) & fixpoint (_move_ring_sat(ring_ -1));
                }
            }
            return res ;
        }
    }
    
    size_t hash() const {
        // a hash function (avoid hash value 0)
        return 6961*(ring_+1);
    }
    
    bool operator==(const StrongHom &s) const {
        _move_ring_sat* ps = (_move_ring_sat*)&s;
        return ring_ == ps->ring_;
    }
    
  _GHom * clone () const {  return new _move_ring_sat(*this); }
};

// to be more pleasant for users  
GHom move_ring_sat ( int ring ) {
    return _move_ring_sat (ring);
}


// generic version no ring specified, just apply to current ring
// builds upon version move_ring_sat
class _move_ring_sat_gen : public StrongHom {
  
public :
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   
  
  GHom phi(int vr, int vl) const {
    // ring reached 
    // try to move to all new positions
    // Initialize res with Id
    GHom res = GHom(vr,vl) ;
    for (int i=0 ; i <NB_POLES ; i++) {
      // test all possible moves from current position = vl
      if (i != vl) {
	// update ring position and test no ring above
	// no_ring_above propagates on the bottom of the DDD ; it returns 0 if preconditions are not met 
	// or a DDD with only paths where the move is legal
	res = (res + ( GHom (vr , i) & _no_ring_above(i , vl) )) & fixpoint (_move_ring_sat_gen());
      }
    }
    return res ;
  }
  
  
  size_t hash() const {
    return 6961;
  }
  
  bool operator==(const StrongHom&) const {
    return true;
  }
  
  _GHom * clone () const {  return new _move_ring_sat_gen(*this); }
};

// to be more pleasant for users  
GHom move_ring_sat_gen ( ) {
  return _move_ring_sat_gen ();
}


// generic version no ring specified, just apply to current ring
class _move_ring_explicit_sat : public StrongHom {
  
public :
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   
  
  GHom phi(int vr, int vl) const {
    // ring reached 
    // try to move to all new positions
    // Initialize res with Id
    GHom res = GHom(vr,vl) ;
    for (int i=0 ; i <NB_POLES ; i++) {
      // test all possible moves from current position = vl
      if (i != vl) {
	// first of all saturate successor node then
	// update ring position and test no ring above
	// no_ring_above propagates on the bottom of the DDD ; it returns 0 if preconditions are not met 
	// or a DDD with only paths where the move was legal
	// Additionnally we resaturate the results of this test before using them
	res = (res + ( GHom (vr , i) & saturate() & _no_ring_above(i , vl) )) & saturate()  ;
      }
    }
    return res ;
  }
  
  size_t hash() const {
    return 6961;
  }
  
  bool operator==(const StrongHom&) const {
    return true;
  }
    _GHom * clone () const {  return new _move_ring_explicit_sat(*this); }
};

// to be more pleasant for users  
GHom move_ring_explicit_sat ( ) {
  return _move_ring_explicit_sat ();
}

// "saturate" fires all events that can be fired from a given node to
//  the leaves and returns a saturated node (à la Ciardo's RecFireAndSat).
GHom saturate () {
  return fixpoint(move_ring_explicit_sat());
}
  

/// SDD homomorphism of the transition relation
class transition_relation : public StrongShom {
 public :
  transition_relation() {};

  GSDD phiOne () const {
    return GSDD::one;
  }

  GShom phi(int vr, const DataSet & vl) const {
    // very basic, simply saturate arc value with move_ring
    // we know there is only one level of depth, therefore DataSet concrete type is DDD
    DDD sat =  saturate() ( (const DDD &) vl);
    return  GShom ( vr, sat) ;
  }

  size_t hash() const {
    return  61907 ;
  }

  bool operator==(const StrongShom&) const {
     return  true;
  }  
  _GShom * clone () const {  return new transition_relation(*this); }
}; 

// SDD application
GShom saturateSDD_singleDepth () { 
  return transition_relation() ;
}


/////////////////////////////////
//////// Working with SDD and IntDataSet


 
// Removes any path such that one of the variables takes value i or value j
// Not test on variable number means this operation should be used from "mid-height"
class _no_ring_above_intdata : public StrongShom {
  // the 2 poles that have to be clear
  IntDataSet set;
public :
  _no_ring_above_intdata (int i, int j) { 
    // construct from vector
    std::vector<int> v (2);
    v[0] = i ;
    v[1] = j;
    set = IntDataSet(v);
  }

  GSDD phiOne() const {
    return GSDD::one;
  }     
  
  // reject any path with ANY ring that is on pole i or pole j
  GShom phi(int vr, const DataSet & vl) const {
    // we know there is only one level of depth, therefore DataSet concrete type is IntDataSet
    DataSet * tofree =  vl.set_minus(set);
    IntDataSet res ( *( (IntDataSet *) tofree ) );
    delete tofree;

    if (! res.empty()) {
      // propagate this test AND (re)saturate resulting nodes
      return GShom(vr,res, saturateSDD_IntData() &GShom(this));
    } else {
      // cut this branch and exploration
      return GSDD::null;
    }
  }

  size_t hash() const {
    return set.set_hash();
  }

  bool operator==(const StrongShom &s) const {
    _no_ring_above_intdata* ps = (_no_ring_above_intdata*)&s;
    return set.set_equal(ps->set );
  }

  _GShom * clone () const {  return new _no_ring_above_intdata(*this); }  
};


// generic version no ring specified, just apply to current ring
class _move_ring_intdata : public StrongShom {
  
public :
  
  GSDD phiOne() const {
    return GSDD::one;
  }                   
  
  GShom phi(int vr, const DataSet& vl) const {
    // ring reached 
    // try to move to all new positions
    // Initialize res with Id
    GShom res = GShom(vr,vl) ;
    for (IntDataSet::const_iterator vlit = ((const IntDataSet&)vl).begin() ; vlit != ((const IntDataSet&)vl).end() ; ++vlit ) {
      for (int i=0 ; i <NB_POLES ; i++) {
	// test all possible moves from current position = vl
	if (i != *vlit) {
	  // first of all saturate successor node then
	  // update ring position and test no ring above
	  // no_ring_above_intdata propagates on the bottom of the SDD ; it returns 0 if preconditions are not met 
	  // or an SDD with only paths where the move was legal
	  // Additionnally we resaturate the results of this test before using them
	  res = (res + ( GShom (vr , IntDataSet(std::vector<int> (1,i)) ) & saturateSDD_IntData() & _no_ring_above_intdata(i , *vlit) )) & saturateSDD_IntData()  ;
	}
      }
    }
    return res ;
  }
  
  size_t hash() const {
    return 6961;
  }
  
  bool operator==(const StrongShom&) const {
    return true;
  }
  
  _GShom * clone () const {  return new _move_ring_intdata(*this); }
};

// to be more pleasant for users  
GShom move_ring_intdata ( ) {
  return  _move_ring_intdata ();
}

// "saturate" fires all events that can be fired from a given node to
//  the leaves and returns a saturated node (à la Ciardo's RecFireAndSat).
GShom saturateSDD_IntData () {
  return fixpoint(move_ring_intdata());
}
