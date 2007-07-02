#ifndef NAT_PLUS_ZERO__HH
#define NAT_PLUS_ZERO__HH


// class _no_ring_above : public StrongShom {
//   // the 2 poles that have to be clear
//   IntDataSet set;
// public :
//   _no_ring_above (int i, int j) { 
//     // construct from vector
//     vector<int> v (2);
//     v[0] = i ;
//     v[1] = j;
//     set = IntDataSet(v);
//   }

//   GSDD phiOne() const {
//     return GSDD::one;
//   }     
  
//   // reject any path with ANY ring that is on pole i or pole j
//   GShom phi(int vr, const DataSet & vl) const {
//     // we know there is only one level of depth, therefore DataSet concrete type is IntDataSet
//     DataSet * tofree =  vl.set_minus(set);
//     IntDataSet res ( *( (IntDataSet *) tofree ) );
//     delete tofree;

//     if (! res.empty()) {
//       // propagate this test AND (re)saturate resulting nodes
//       return GShom(vr,res, saturate() &GShom(this));
//     } else {
//       // cut this branch and exploration
//       return GSDD::null;
//     }
//   }

//   size_t hash() const {
//     return set.set_hash();
//   }

//   bool operator==(const StrongShom &s) const {
//     _no_ring_above* ps = (_no_ring_above*)&s;
//     return set.set_equal(ps->set );
//   }
  
// };

// // generic version no ring specified, just apply to current ring
// class _move_ring : public StrongShom {
  
// public :
  
//   GSDD phiOne() const {
//     return GSDD::one;
//   }                   
  
//   GShom phi(int vr, const DataSet& vl) const {
//     // ring reached 
//     // try to move to all new positions
//     // Initialize res with Id
//     GShom res = GShom(vr,vl) ;
//     for (IntDataSet::const_iterator vlit = ((const IntDataSet&)vl).begin() ; vlit != ((const IntDataSet&)vl).end() ; ++vlit ) {
//       for (int i=0 ; i <NB_POLES ; i++) {
// 	// test all possible moves from current position = vl
// 	if (i != *vlit) {
// 	  // first of all saturate successor node then
// 	  // update ring position and test no ring above
// 	  // no_ring_above propagates on the bottom of the SDD ; it returns 0 if preconditions are not met 
// 	  // or an SDD with only paths where the move was legal
// 	  // Additionnally we resaturate the results of this test before using them
// 	  res = (res + ( GShom (vr , IntDataSet(vector<int> (1,i)) ) & saturate() & new _no_ring_above(i , *vlit) )) & saturate()  ;
// 	}
//       }
//     }
//     return res ;
//   }
  
//   size_t hash() const {
//     return 6961;
//   }
  
//   bool operator==(const StrongShom &s) const {
//     return true;
//   }
  
// };

// // to be more pleasant for users  
// GShom move_ring ( ) {
//   return new _move_ring ();
// }

// // "saturate" fires all events that can be fired from a given node to
// //  the leaves and returns a saturated node (à la Ciardo's RecFireAndSat).
// GShom saturate () {
//   return fixpoint(move_ring());
// //  return move_ring();
// }

 

#endif // NAT_PLUS_ZERO__HH
