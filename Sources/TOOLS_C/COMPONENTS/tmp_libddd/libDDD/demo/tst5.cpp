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

/********************************************************/
/* Define the strongHom _SetVar : var2 = var1           */
/* using 3 strongHoms : _SetCst, _SetVarUp, _SetVarDown */
/********************************************************/

class _SetCst:public StrongHom {
  int var, val;
public:
  _SetCst(int vr, int vl):var(vr),val(vl) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (var == vr)
      return GHom(vr,val);
    else 
      return GHom(vr,vl,GHom(this)); 
  }

  size_t hash() const {
    return var+val;
  }

  bool operator==(const StrongHom &s) const {
    _SetCst* ps = (_SetCst*)&s;
    return var == ps->var && val == ps->val;
  }
};

// **********************************************
class _SetVarUp:public StrongHom {
  int var, val; 
public:
  _SetVarUp(int vr, int vl):var(vr),val(vl) {}

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    return GHom(vr,vl,GHom(var,val)); 
  }

  size_t hash() const {
    return var+val;
  }

  bool operator==(const StrongHom &s) const {
    _SetVarUp* ps = (_SetVarUp*)&s;
    return var == ps->var && val == ps->val;
  }
};

// **********************************************
class _SetVarDown:public StrongHom {
  int var1, var2; // var1 <= var2
public:
  _SetVarDown(int v1, int v2):var1(v1),var2(v2) {}

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    if (var2 == vr)
      return GHom(var1,vl,GHom(vr,vl)); 
    else
      return GHom(new _SetVarUp(vr,vl))&(GHom(this));
  }

  size_t hash() const {
    return var1+var2;
  }

  bool operator==(const StrongHom &s) const {
    _SetVarDown* ps = (_SetVarDown*)&s;
    return var1 == ps->var1 && var2 == ps->var2;
  }
};

// **********************************************
class _SetVar:public StrongHom {
  int var1, var2; // var1 <= var2
public:
  _SetVar(int v1, int v2):var1(v1),var2(v2) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (var2 == vr)
      return GHom(vr,vl,GHom(new _SetCst(var1,vl))); 
    if (var1 != vr)
      return GHom(vr,vl,GHom(this));
    return GHom(new _SetVarDown(var1, var2));
  }

  size_t hash() const {
    return var1+var2;
  }

  bool operator==(const StrongHom &s) const {
    _SetVar* ps = (_SetVar*)&s;
    return var1 == ps->var1 && var2 == ps->var2;
  }
};

// User function : Construct a Hom for a Strong Hom _SetVar
GHom setCst(int var,int val){return new _SetCst(var,val);};
GHom setVar(int var1,int var2){return new _SetVar(var1,var2);};

int main(){
  initName();

  cout <<"****************"<<endl;
  cout <<"* Define DDD u *"<<endl;
  cout <<"****************"<<endl;

  DDD a=DDD(A,1,DDD(B,3))+DDD(A,4,DDD(B,4));
  DDD b=DDD(C,6,DDD(D,1))+DDD(C,2,DDD(D,2));
  DDD u=a^b;
  cout <<"u="<< endl<<u<<endl;

  cout <<"****************************************************"<<endl;
  cout <<"* Strong Hom <X=c> : set the first value of X to d *"<<endl;
  cout <<"****************************************************"<<endl;

  Hom setC_5 = setCst(C,5);
  cout <<"<C=5>(u)="<< endl<<setC_5(u)<<endl;

  cout <<"***********************************************************************"<<endl;
  cout <<"* Strong Hom <X=Y> : set the first value of Y to the first value of Y *"<<endl;
  cout <<"***********************************************************************"<<endl;

  Hom setC_A = setVar(C,A);
  cout <<"<C=A>(u)="<< endl<<setC_A(u)<<endl;

  Hom setA_C = setVar(A,C);
  cout <<"<A=C>(u)="<< endl<<setA_C(u)<<endl;

  Hom setB_D = setVar(B,D);
  cout <<"<B=D>(u)="<< endl<<setB_D(u)<<endl;

  cout <<"<C=5><A=C><B=D>(u)="<< endl<<(setC_5&setA_C&setB_D)(u)<<endl;

  MemoryManager::pstats();
  
  return 1;
}



