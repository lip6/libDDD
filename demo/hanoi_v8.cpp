/** An example resolution of the famous towers of Hanoi puzzle. *
 * v8 : This variant exhibits the use IntDataSet based SDD, emphasizing the differences with DDD.
*/


#include <cstring>
#include <string>
#include <iostream>
using namespace std;

#include "IntDataSet.h"
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

// predeclaration
GShom saturate ();
  
// Removes any path such that one of the variables takes value i or value j
// Not test on variable number means this operation should be used from "mid-height"
class _no_ring_above : public StrongShom {
  // the 2 poles that have to be clear
  IntDataSet set;
public :
  _no_ring_above (int i, int j) { 
    // construct from vector
    vector<int> v (2);
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
    DataSet * res =  vl.set_minus(set);

    /////////// Memory leak BUUGGGGG : res is not freed

    if (! res->empty()) {
      // propagate this test AND (re)saturate resulting nodes
      return GShom(vr,*res, saturate() &GShom(this));
    } else {
      // cut this branch and exploration
      return GSDD::null;
    }
  }

  size_t hash() const {
    return set.set_hash();
  }

  bool operator==(const StrongShom &s) const {
    _no_ring_above* ps = (_no_ring_above*)&s;
    return set.set_equal(ps->set );
  }
  
};

// generic version no ring specified, just apply to current ring
class _move_ring : public StrongShom {
  
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
	  // no_ring_above propagates on the bottom of the SDD ; it returns 0 if preconditions are not met 
	  // or an SDD with only paths where the move was legal
	  // Additionnally we resaturate the results of this test before using them
	  res = (res + ( GShom (vr , IntDataSet(vector<int> (1,i)) ) & saturate() & new _no_ring_above(i , *vlit) )) & saturate()  ;
	}
      }
    }
    return res ;
  }
  
  size_t hash() const {
    return 6961;
  }
  
  bool operator==(const StrongShom &s) const {
    return true;
  }
  
};

// to be more pleasant for users  
GShom move_ring ( ) {
  return new _move_ring ();
}

// "saturate" fires all events that can be fired from a given node to
//  the leaves and returns a saturated node (à la Ciardo's RecFireAndSat).
GShom saturate () {
  return fixpoint(move_ring());
//  return move_ring();
}
  
  
int main(int argc, char **argv){
  if (argc == 2) {
    NB_RINGS = atoi(argv[1]);
  }

  // Define a name for each variable
  initName();

  // The initial state
  // User program variables should be DDD not GDDD, to prevent their garbage collection
  SDD M0 = GSDD::one ;
  // construct an initial state for the problem, all rings are on pole 0
  IntDataSet s (vector<int> (1,0) );
  for (int i=0; i<NB_RINGS ; i++ ) {
    // note the use of left-concat (adding at the top of the structure), 
    M0 = SDD(i, s, M0);

    // expression is equivalent to : DDD(i,0) ^ MO
    // less expensive than right-concat which forces to recanonize nodes
    // would be written : for ( i--) M0 = M0 ^ DDD(i,0);
  }

  cout << M0 << endl ;

  // Consider one single saturate event that recursively fires all events 
  // Saturate topmost node <=> reach fixpoint over transition relation
  SDD ss =  saturate() (M0) ;

  cout << ss << endl ;
  // stats
  cout << "Number of states : " << ss.nbStates() << endl ;
  cout << "DDD Final/Peak nodes : " << ss.node_size().second << "/" << DDD::peak() << endl;
  cout << "SDD Final/Peak nodes : " << ss.node_size().first << "/" << SDD::peak() << endl;
  cout << "Cache entries DDD/SDD : " << MemoryManager::nbDED() <<  "/" <<  MemoryManager::nbSDED() << endl ;
//  MemoryManager::pstats();
}
