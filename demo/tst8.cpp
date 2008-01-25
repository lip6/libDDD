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

typedef enum {A, B, C, D,E, F, G} var;
var variables;
const char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}


// Increment the first value of var
class _sub:public StrongHom {
  int var;
  GHom f;
public:
  _sub(int vr,const GHom &g):var(vr),f(g) {};

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    if (vr == var)
      return f&GHom(vr,vl);
    else 
      return GHom(vr,vl,GHom(this)); 
  }

  size_t hash() const {
    return (size_t) var+__gnu_cxx::hash<GHom>()(f);
  }

  bool operator==(const StrongHom &s) const {
    _sub* ps = (_sub*)&s;
    return (var == ps->var)&&(f == ps->f);
  }

  void mark(){
    MemoryManager::mark(f);
  }
};

// User function : Construct a Hom for a Strong Hom _plusplus
GHom sub(int vr,const GHom &g){return new _sub(vr,g);};

int main(){
  initName();

  cout <<"****************"<<endl;
  cout <<"* Define DDD u *"<<endl;
  cout <<"****************"<<endl;

  DDD a=DDD(A,1,DDD(A,1));
  DDD b=DDD(C,1,DDD(A,1))+DDD(C,2,DDD(B,3));
  DDD u=a^b;
  Hom h(C,43);
  cout <<"u="<< endl<<u<<endl;

  cout <<"******************************************************"<<endl;
  cout <<"* Strong Hom <X++> : incremente the first value of X *"<<endl;
  cout <<"******************************************************"<<endl;

  MemoryManager::garbage();
  
  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl<<endl;

  Hom fa = sub(A,h);
  Hom fb = sub(B,h);
  Hom fc = sub(C,h);
  Hom fd = sub(D,h);

  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl<<endl;

  cout <<"<A>(u)="<< endl<<fa(u)<<endl;
  cout <<"<B>(u)="<< endl<<fb(u)<<endl;
  cout <<"<C>(u)="<< endl<<fc(u)<<endl;
  cout <<"<D>(u)="<< endl<<fd(u)<<endl<<endl;

  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl<<endl;

  MemoryManager::garbage();
  
  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl;
  MemoryManager::pstats();

  return 1;
}


