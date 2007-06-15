/** An example resolution of the famous towers of Hanoi puzzle. *
 *  This variant uses only DDD and no saturation,  *
 *  but starts operations when the ring is reached */


#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"

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
  
};


class _move_ring : public StrongHom {
  // ring we are trying to move
  int ring_;
  
public :
  _move_ring (int ring): ring_(ring) {};
  
  GDDD phiOne() const {
    return GDDD::one;
  }                   
  
  GHom phi(int vr, int vl) const {
    if (vr != ring_ ) {
      // target ring not reached yet : propagate
      return GHom(vr,vl,this) ;
    } else {
      // ring reached 
      // try to move to all new positions
      GHom res = GDDD::null;
      for (int i=0 ; i <NB_POLES ; i++) {
	// test all possible moves from current position = vl
	if (i != vl) {
	  // update ring position and test no ring above
	  // no_ring_above propagates on the bottom of the DDD ; it returns 0 if preconditions are not met 
	  // or a DDD with only paths where the move is legal
	  res = res + ( GHom (ring_ , i) & new _no_ring_above(i , vl) );
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
  
};

// to be more pleasant for users  
GHom move_ring ( int ring ) {
  return new _move_ring (ring);
}
  
  
int main(int argc, char **argv){
  if (argc == 2) {
    NB_RINGS = atoi(argv[1]);
  }

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

  // To store the set of events
  vector<Hom> events;
  for (int i=0 ; i < NB_RINGS ; i++) {
    // Consider one event per ring, it will split into the possible moves when it reaches the ring
    // ie : ev = \Sum_{i \neq j} swap_pole (ring,i,j)
    // with swap_pole defined as in hanoi_v1
    events.push_back(move_ring(i));
  }

  // Fixpoint over events + Id
  DDD ss, tmp = M0;
  do {
    ss = tmp;
    for (vector<Hom>::const_reverse_iterator it = events.rbegin(); it != events.rend(); it++) {
      tmp = tmp + (*it) (tmp);
    }
  } while (ss != tmp);

  // stats
  cout << "Number of states : " << ss.nbStates() << endl ;
  cout << "Final/Peak nodes : " << ss.size() << "/" << DDD::peak() << endl;
  cout << "Cache entries : " << MemoryManager::nbDED() <<endl ;
}
