#include <iostream>
using namespace std;

#include "DDD.h"
#include "Hom.h"
#include "MemoryManager.h"

enum {A, B, C, D,E, F, G} variables;
char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}

// incremente all values of X
class _plusplus:public StrongHom {
  int var;
public:
  _plusplus(int vr):var(vr) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (vr == var)
      return GHom(vr,vl+1)&GHom(this);
    else 
      return GHom(vr,vl)&GHom(this); 
  }

  size_t hash() const {
    return var;
  }

  bool operator==(const StrongHom &s) const {
    _plusplus* ps = (_plusplus*)&s;
    return var == ps->var;
  }
};

// User function : Construct a Hom for a Strong Hom _plusplus
GHom plusplus(int vr){return new _plusplus(vr);};

// selects only paths such that first occurrence of path has value < x
class _selectVarLim:public StrongHom {
  int var;
  int lim;
public:
  _selectVarLim(int vr,int vl):var(vr),lim(vl) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (vr == var)
      if (vl <= lim)
	return GHom(vr,vl);
      else
	return GDDD::null;
    else 
      return GHom(vr,vl,this); 
  }

  size_t hash() const {
    return var*17+lim*23;
  }

  bool operator==(const StrongHom &s) const {
    _selectVarLim* ps = (_selectVarLim*)&s;
    return var == ps->var && lim == ps->lim;
  }
};


// User function : Construct a Hom for a Strong Hom _selectVarLim
GHom selectVarLim(int vr,int lim){return new _selectVarLim(vr,lim);};


int main(){
  initName();

  cout <<"****************"<<endl;
  cout <<"* Define DDD u *"<<endl;
  cout <<"****************"<<endl;

  DDD a=DDD(A,1,DDD(A,1));
  DDD b=DDD(C,1,DDD(A,1))+DDD(C,2,DDD(A,3));
  DDD u=a^b;

  cout <<"u="<< endl<<u<<endl;

  cout <<"**************************************************"<<endl;
  cout <<"* Fixpoint limit increment : increment all values of X *"<<endl;
  cout <<"* up to limit = 6 *"<<endl;
  cout <<"**************************************************"<<endl;

  Hom fc = plusplus(C);
  Hom limc = selectVarLim(C,5);

  Hom satLim = fixpoint( GHom::id + ( plusplus(C) & selectVarLim(C,5)) );

  cout <<"<!C++<=6>(u)="<< endl<<satLim(u)<<endl;

  return 1;

}
