#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "Hom.h"

enum {A, B, C, D,E, F, G} variables;
char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}

/***********************************************************/
/* Define the strongHom Permute : var1,var2 = var2,var1    */
/* using 3 strongHoms : PermuteUp, PermuteDown, PermuteFin */
/***********************************************************/

// **********************************************
class PermuteUp:public StrongHom {
  int var, val; // var1 <= var2
public:
  PermuteUp(int vr, int vl):var(vr),val(vl) {}

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    return GHom(vr,vl,GHom(var,val)); 
  }

  size_t hash() const {
    return var+val+400;
  }

  bool operator==(const StrongHom &s) const {
    PermuteUp* ps = (PermuteUp*)&s;
    return (var == ps->var && val == ps->val);
  }
};

// **********************************************
class PermuteDown:public StrongHom {
  int var, val; // var1 <= var2
public:
  PermuteDown(int vr, int vl):var(vr),val(vl) {}

  GDDD phiOne() const {
    return GDDD::top;
  }                   

  GHom phi(int vr, int vl) const {
    if (var == vr)
      return GHom(0,vl,GHom(vr,val)); 
    if (var != vr)
      return GHom(new PermuteUp(vr,vl))&(GHom(this));
    return GHom::id;
  }
 
  size_t hash() const {
    return var+val+300;
  }

  bool operator==(const StrongHom &s) const {
    PermuteDown* ps = (PermuteDown*)&s;
    return var == ps->var && val == ps->val;
  }
};


// **********************************************
class PermuteFin:public StrongHom {
  int var; // var1 <=> var2
public:
  PermuteFin(int vr):var(vr) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    return GHom(var,vl);
  }

  size_t hash() const {
    return var+200;
  }

  bool operator==(const StrongHom &s) const {
    PermuteFin* ps = (PermuteFin*)&s;
    return var == ps->var;
  }
};

// **********************************************
class Permute:public StrongHom {
  int var1, var2; // var1 <=> var2
public:
  Permute(int v1, int v2):var1(v1),var2(v2) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (var1 == vr)
      return GHom(new PermuteFin(var1))&(GHom(new PermuteDown(var2,vl))); 
    if (var2 == vr)
      return GHom(new PermuteFin(var2))&(GHom(new PermuteDown(var1,vl))); 
    return GHom(vr,vl,GHom(this));
  }

  size_t hash() const {
    return var1+var2+100;
  }

  bool operator==(const StrongHom &s) const {
    Permute* ps = (Permute*)&s;
    return var1 == ps->var1 && var2 == ps->var2;
  }
};

// User function : Construct a Hom for a Strong Hom _SetVar
GHom permute(int var,int val){return new Permute(var,val);};

int main(){
  initName();

  cout <<"****************"<<endl;
  cout <<"* Define DDD u *"<<endl;
  cout <<"****************"<<endl;

  DDD a=DDD(A,1,DDD(B,3))+DDD(A,4,DDD(B,4));
  DDD b=DDD(C,6,DDD(D,1))+DDD(C,2,DDD(D,2));
  DDD u=a^b;
  cout <<"u="<< endl<<u<<endl;

  cout <<"**************************************************************"<<endl;
  cout <<"* Strong Hom <X,Y=Y,X> : permute the first value of X and Y *"<<endl;
  cout <<"**************************************************************"<<endl;

  Hom permuteA_C = permute(A,C);
  DDD v=permuteA_C(u);
  cout <<"<A,C=C,A>(u)="<< endl<<v<<endl;

  return 1;
}



