#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
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

int main(){
  initName();

  cout <<"****************"<<endl;
  cout <<"* Define DDD u *"<<endl;
  cout <<"****************"<<endl;

  DDD a=DDD(A,1,DDD(A,1));
  DDD b=DDD(C,1,DDD(A,1))+DDD(C,2,DDD(B,3));
  DDD u=a^b;

  cout <<"u="<< endl<<u<<endl;

  cout <<"**************************************************"<<endl;
  cout <<"* Strong Hom <!X++> : incremente all values of X *"<<endl;
  cout <<"**************************************************"<<endl;

  Hom fa = plusplus(A);
  Hom fb = plusplus(B);
  Hom fc = plusplus(C);
  Hom fd = plusplus(D);

  cout <<"<!A++>(u)="<< endl<<fa(u)<<endl;
  cout <<"<!B++>(u)="<< endl<<fb(u)<<endl;
  cout <<"<!C++>(u)="<< endl<<fc(u)<<endl;
  cout <<"<!D++>(u)="<< endl<<fd(u)<<endl;

  MemoryManager::pstats();

  return 1;
}
