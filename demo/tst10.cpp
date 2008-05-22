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

#include <iostream>
using namespace std;
// for ::hash functions
// using namespace __gnu_cxx;

#include "DDD.h"
#include "Hom.h"
#include "MemoryManager.h"
#include "PlusPlus.hh"

typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}



/// This Hom seeks a variable and applies its argument homomorphism when found.
class _seek:public StrongHom {
  /// the targeted var
  int var;
  /// the embedded homomorphism
  GHom h;
public :
  /// constructor
  /// \param vr the target variable's index
  /// \param hh the homomorphism to apply, either a simple hom, or a fixpoint construct
  _seek(int vr, const GHom & hh):var(vr),h(hh) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (vr == var)
      return h & GHom(vr,vl);
    else 
      return GHom(vr,vl,this); 
  }

  size_t hash() const {
    return var ^ h.hash();
  }

  bool operator==(const StrongHom &s) const {
    _seek* ps = (_seek*)&s;
    return var == ps->var && h==ps->h;
  }
  _GHom * clone () const {  return new _seek(*this); }
};  


/// selects only paths such that first occurrence of path has value < x.
/// i.e if next value on arc is strictly superior to lim, path is destroyed.
class _selectVarLim:public StrongHom {
  /// the limit tested against
  int lim;
public:
  /// constructor
  _selectVarLim(int vl):lim(vl) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
      if (vl <= lim)
	return GHom(vr,vl);
      else
	return GDDD::null;
  }

  size_t hash() const {
    return lim*23;
  }

  bool operator==(const StrongHom &s) const {
    _selectVarLim* ps = (_selectVarLim*)&s;
    return lim == ps->lim;
  }

  _GHom * clone () const {  return new _selectVarLim(*this); }
};


// User function : Construct a Hom for a Strong Hom _selectVarLim
GHom selectVarLim(int lim){return _selectVarLim(lim);};

/// This example is built to show how superior saturation using fixpoint is to breadth first search.
/// It also gives a simple saturation example.
int main(){
  initName();
  
  
  
  cout <<"****************"<<endl;
  cout <<"* Define DDD u *"<<endl;
  cout <<"****************"<<endl;
  
  // This segment is a few variables long, it just increases the depth of the variable we want to work on (the last one).   
  DDD longSegment = DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1))))))));
  // an element in the example construct
  DDD a=DDD(A,1,DDD(A,1));  
  // an element in the example construct
  DDD b=DDD(B,1,DDD(C,1))+DDD(B,2,DDD(C,3));
  // an element in the example construct
  DDD c=DDD(A,1,DDD(A,2))^DDD(B,2,DDD(C,2)) ;
  // the value we work on
  DDD u=longSegment^( (a^b) + c );
	  
  cout <<"u="<< endl<<u<<endl;

  /// instantiate a plusplus hom
  Hom fc = plusplusFirst();
  /// chooose the target limit
  Hom limc = selectVarLim(39);

  /// this block tests the fixpoint version
  {
  cout <<"**************************************************"<<endl;
  cout <<"* Fixpoint limit increment : increment all values of C *"<<endl;
  cout <<"* up to limit = 40, using an embedded fixpoint in saturation manner  *"<<endl;
  cout <<"**************************************************"<<endl;

  Hom satLim = fixpoint( GHom::id + ( fc & limc) );
  Hom full = GHom(_seek(C,satLim));

  cerr <<"<!C++<=6>(u)="<< endl<<full(u).nbStates()<<endl;
  }
  /// print and clear stats for next run
  DDD::pstats(true);
  DED::pstats(true);
  MemoryManager::garbage();

  /// this block shows a BFS version
  {
  cout <<"**************************************************"<<endl;
  cout <<"* BFS limit increment : increment all values of C *"<<endl;
  cout <<"* up to limit = 40, using a naive \"iterate until fixpoint\" strategy *"<<endl;
  cout <<"**************************************************"<<endl;

  Hom full = GHom(_seek(C,fc & limc));

  /// the external fixpoint iteration
  DDD v = u;
  do {
    v = u;
    u = u + full (u);
  }  while (u!=v) ;
  
  cerr <<"<!C++<=6>(u)="<< endl<<u.nbStates()<<endl;
  }
  /// show stats
  DDD::pstats(true);
  DED::pstats(true);
  MemoryManager::garbage();

  return 1;

}
