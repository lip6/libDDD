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

#include "DDD.h"
#include "DED.h"
#include "Hom.h"
#include "MemoryManager.h"
#include "statistic.hpp"

typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}


// Apply the hom f to the first occurrence of var
class _sub:public StrongHom {
  int var;
  // We use the type GHom (instead of Hom) in the attributes of homomorphisms.
  // This is necessary to allow correct memory reference counting.
  // Similarly use an attribute of type GDDD instead of DDD.
  // When using such attributes you must implement mark().
  GHom f;
public:
  _sub(int vr,const GHom &g):var(vr),f(g) {};

  GDDD phiOne() const {
    return GDDD::top;
  }      

  bool skip_variable (int vr) const {
    /** this definition is equivalent to placing :
	if (vr != var)
	   return GHom(vr,vl,this); 
    * at the top of phi() function. 
    * However it enables library optimizations when defined as "skip".*/
    return var != vr;
  }

  GHom phi(int vr, int vl) const {
    // implicitly vr == var because of skip_variable definition.
    return f&GHom(vr,vl);
  }

  size_t hash() const {
    return (size_t) var + f.hash();
  }

  bool operator==(const StrongHom &s) const {
    const _sub & ps = (const _sub&)s;
    return (var == ps.var)&&(f == ps.f);
  }

  void mark(){
    // must implement mark function if attributes of type GHom or GDDD
    f.mark();
  }
};

// User function : Construct a Hom to avoid user manipulation of "new..."
GHom applyToVar (int vr,const GHom &g){return new _sub(vr,g);};

int main(){
  initName();

  cout <<"****************"<<endl;
  cout <<"* Define DDD u *"<<endl;
  cout <<"****************"<<endl;

  DDD a=DDD(A,1,DDD(A,1));
  DDD b=DDD(C,1,DDD(A,1))+DDD(C,2,DDD(B,3));
  DDD u=a^b;
  cout <<"u="<< endl<<u<<endl;
  // left concatenates a constant : h(d) = C-43->d
  Hom h(C,43);

  Statistic s(u,"initial",CSV);
  s.print_header(cout);
  s.print_line(cout);

  Hom fa = applyToVar(A,h);
  Hom fb = applyToVar(B,h);
  Hom fc = applyToVar(C,h);
  Hom fd = applyToVar(D,h);


  cout <<"<A>(u)="<< endl<<fa(u)<<endl;
  cout <<"<B>(u)="<< endl<<fb(u)<<endl;
  cout <<"<C>(u)="<< endl<<fc(u)<<endl;
  cout <<"<D>(u)="<< endl<<fd(u)<<endl<<endl;

  Statistic s2(u,"after apply",CSV);
  s2.print_line(cout);
  MemoryManager::garbage();
  Statistic s3(u,"after garbage",CSV);
  s3.print_line(cout);
  s3.print_trailer(cout);
  return 1;
}


