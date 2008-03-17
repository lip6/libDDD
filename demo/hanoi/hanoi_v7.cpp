/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2008 Yann Thierry-Mieg, Jean-Michel Couvreur      */
/*                             and Denis Poitrenaud                         */
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

/** An example resolution of the famous towers of Hanoi puzzle. *
 *  v7 : This variant introduces the use of SDD  *
 *  A single SDD variable is used, the program builds upon v6. *
*/


#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"
#include "init.hh"

// we use one DDD variable per ring, ring 0 is the topmost, 
// and is stored at the bottom of the DDD
static int NB_RINGS= 5;
// Each variable domain is {0,1,2} expressing the pole the variable is on
static int NB_POLES= 3;

void initName() {
  char buff [12];
  for (int i=0; i< NB_RINGS; i++) {
    sprintf(buff,"ring %d",i);
    DDD::varName(i,buff);
  }
}

// predeclaration
GHom saturate ();
// SDD application
GShom saturateSDD ();

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
}; 

// SDD application
GShom saturateSDD () { 
  return new transition_relation() ;
}
  
// Removes any path such that one of the variables takes value i or value j
// Not test on variable number means this operation should be used from "mid-height"
class _no_ring_above : public StrongHom {
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
  GHom phi(int vr, int vl) const {
    if ( vl == i_ || vl == j_ )
      // cut this branch and exploration
      return GDDD::null;
    else
      // propagate this test AND (re)saturate resulting nodes
      return GHom(vr,vl, saturate() &GHom(this));
  }

  size_t hash() const {
    return (i_ &  j_<<16)  *  9749;
  }

  bool operator==(const StrongHom &s) const {
    	_no_ring_above* ps = (_no_ring_above*)&s;
	return i_ == ps->i_ && j_ == ps->j_ ;
  }
  
};

// generic version no ring specified, just apply to current ring
class _move_ring : public StrongHom {
  
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
	res = (res + ( GHom (vr , i) & saturate() & new _no_ring_above(i , vl) )) & saturate()  ;
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
  
};

// to be more pleasant for users  
GHom move_ring ( ) {
  return new _move_ring ();
}

// "saturate" fires all events that can be fired from a given node to
//  the leaves and returns a saturated node (à la Ciardo's RecFireAndSat).
GHom saturate () {
  return fixpoint(move_ring());
}
  
  
int main(int argc, char **argv){
  if (argc == 2) {
    NB_RINGS = atoi(argv[1]);
  }

 	d3::init init;

  // Define a name for each variable
  initName();

  // The initial state
  // User program variables should be DDD not GDDD, to prevent their garbage collection
  DDD M0 = GDDD::one ;
  // construct an initial state for the problem, all rings are on pole 0
  for (int i=0; i<NB_RINGS ; i++ ) {
    // note the use of left-concat (adding at the top of the structure), 
    M0 = DDD(i,0, M0);
    // expression is equivalent to : DDD(i,0) ^ MO
    // less expensive than right-concat which forces to recanonize nodes
    // would be written : for ( i--) M0 = M0 ^ DDD(i,0);
  }

  // Add an SDD external var, bearing number 0
  SDD M1 = SDD ( 0 , M0 ) ;

  // Consider one single saturate event that recursively fires all events 
  // Saturate topmost node <=> reach fixpoint over transition relation
  SDD ss =  saturateSDD() (M1) ;

  // stats
  cout << "Number of states : " << ss.nbStates() << endl ;
  cout << "DDD Final/Peak nodes : " << ss.node_size().second << "/" << DDD::peak() << endl;
  cout << "SDD Final/Peak nodes : " << ss.node_size().first << "/" << SDD::peak() << endl;
  cout << "Cache entries DDD/SDD : " << MemoryManager::nbDED() <<  "/" <<  MemoryManager::nbSDED() << endl ;
}
