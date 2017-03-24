/****************************************************************************/
/*								            */
/* This file is part of libDDD, a library for manipulation of DDD and SDD.  */
/*     						                            */
/*     Copyright (C) 2001-2009 Yann Thierry-Mieg, Jean-Michel Couvreur      */
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
* v8 : This variant exhibits the use IntDataSet based SDD, emphasizing the differences with DDD.
*/

#include <cmath>
#include <cstring>
#include <string>
#include <iostream>
#include <cstdio>
  using namespace std;

#include "ddd/IntDataSet.h"
#include "ddd/DDD.h"
#include "ddd/DED.h"
#include "ddd/MemoryManager.h"
#include "ddd/statistic.hpp"

// int -> string
std::string toString (int i) {
  char buff [16];
  sprintf (buff,"%d",i);
  return buff;
}

// we use one DDD variable per ring, ring 0 is the topmost,
// and is stored at the bottom of the DDD
static int NB_RINGS= 3;
// Each variable domain is {0,1,2} expressing the pole the variable is on
static int NB_POLES= 3;

#define VAR_STATES 0
#define VAR_HIER 1

///////////////////////////////////////////////////////////////////////////////////////////////////////

void initName() {
  char buff [12];
  for (int i=0; i< NB_RINGS; i++) {
    sprintf(buff,"ring %d",i);
    DDD::varName(i,buff);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

// predeclaration
GShom saturate ();

///////////////////////////////////////////////////////////////////////////////////////////////////////

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
    if (vr == VAR_STATES) {
      // we know there is only one level of depth, therefore DataSet concrete type is IntDataSet
      DataSet * tofree =  vl.set_minus(set);
      IntDataSet res ( *( (IntDataSet *) tofree ) );
      delete tofree;

      // test is useless, if res is empty SDD canonization of GShom(vr,res)(GSDD::one) returns GSDD::null node
      if (! res.empty()) {
  // usually we should
  // propagate this test AND (re)saturate resulting nodes
  // return GShom(vr,res, saturate() &GShom(this));
  // but in fact successor should be GSDD::one so we know the result
        return GShom(vr,res);
      } else {
  // cut this branch and exploration
        return GSDD::null;
      }
    } else {
      // propagate twice
      SDD vl2 = GShom(this) ((const SDD &) vl);
      return GShom (vr , vl2 ) & this ;
    }
  }

  size_t hash() const {
    return set.set_hash();
  }

  bool operator==(const StrongShom &s) const {
    _no_ring_above* ps = (_no_ring_above*)&s;
    return set.set_equal(ps->set );
  }

  _GShom * clone () const {  return new _no_ring_above(*this); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

// generic version no ring specified, just apply to current ring
// move constrained to use 2 specific poles
class _move_ring : public StrongShom {
  // the poles used
  int p1,p2;
  public :

  _move_ring(int pp1,int pp2): p1(pp1),p2(pp2) { assert (p1<p2); };

  GSDD phiOne() const {
    return GSDD::one;
  }

  GShom phi(int vr, const DataSet& vl) const {
    // ring reached
    // try to move to all new positions

    if (vr == VAR_STATES) {
      // Initialize res with Id
      GShom res = GShom(vr,vl) ;

      // concrete level reached : vl is an IntDataSet
      for (IntDataSet::const_iterator vlit = ((const IntDataSet&)vl).begin() ; vlit != ((const IntDataSet&)vl).end() ; ++vlit ) {
        if (*vlit == p1) {
    // move to p2
          res = res +  GShom (vr , IntDataSet(vector<int> (1,p2)));
        } else if (*vlit == p2) {
    // move to p1
          res = res +  GShom (vr , IntDataSet(vector<int> (1,p1)));
        }
      }
      return res ;
    } else {
      // hierarchy, vl is SDD
      SDD vl2 = GShom(this) ((const SDD &) vl);
      return (GShom (vr , vl2 ) & _no_ring_above(p1 , p2))  + (GShom (vr , vl) & (this + GShom::id) );
    }

  }


  size_t hash() const {
    return 6961*p1+p2;
  }

  bool operator==(const StrongShom &s) const {
    return p1== ((const _move_ring &) s).p1 &&  p2== ((const _move_ring &) s).p2;
  }
  _GShom * clone () const {  return new _move_ring(*this); }

};

///////////////////////////////////////////////////////////////////////////////////////////////////////

// to be more pleasant for users
GShom move_ring (int i, int j ) {
  return _move_ring (i,j);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

// "saturate" fires all events that can be fired from a given node to
//  the leaves and returns a saturated node (à la Ciardo's RecFireAndSat).
GShom saturate () {
  GShom moves = GShom::id ;
  for (int i=0; i < NB_POLES ; i++) {
    for (int j=i+1; j < NB_POLES ; j++) {
      moves = moves + move_ring(i,j);
    }
  }
  return fixpoint(moves);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv){
  if (argc == 2) {
    NB_RINGS = atoi(argv[1]);
  }

  // Define a name for each variable
  initName();

  // The initial state
  // construct an initial state for the problem, all rings are on pole 0
  IntDataSet s (vector<int> (1,0) );
  // User program variables should be DDD not GDDD, to prevent their garbage collection
  SDD M0 = SDD(VAR_STATES, s);

  for (int i=0; i<NB_RINGS ; i++ ) {
    // tricky, recursive embedding
    M0 = SDD(VAR_HIER ,M0, SDD(VAR_HIER ,M0) );
  }
  //  cout << M0 << endl ;

  // Consider one single saturate event that recursively fires all events
  // Saturate topmost node <=> reach fixpoint over transition relation
  SDD ss =  saturate() (M0) ;

 // stats
  Statistic S = Statistic(ss,"hanoiv9." + toString(pow((double)2,(double)NB_RINGS)) + "." + toString(NB_POLES),CSV);
  S.print_header(std::cout);
  S.print_line(std::cout);
}
