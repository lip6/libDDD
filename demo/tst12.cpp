#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"
#include "dotExporter.h"


typedef enum {A, dA, B, dB, C, dC} var;
var variables;
char* vn[]= {"A", "dA", "B", "dB", "C", "dC"};


void initName() {
  for (int i=A; i<=dC; i++)
    DDD::varName(i,vn[i]);
}


/// increment the value of the next variable
class _plusplus:public StrongHom {
  int var;
public:
  _plusplus(int var_) :var(var_){}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (vr == var)
      return GHom(vr,vl+1);
    else 
      return GHom(vr,vl,this);
  }

  size_t hash() const {
    return 23*var;
  }

  bool operator==(const StrongHom &s) const {
    return var == ((const _plusplus &)s).var;
  }
};

/// User function : Construct a Hom for a Strong Hom _plusplus
GHom plusplus(int v){return new _plusplus(v);};


/// push distance down
class _push:public StrongHom {
public:
  _push(){}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (vr == dA )
      if (vl == 2)
	return GHom(vr,vl-1, plusplus(dB));
      else 
	return GHom(vr,vl);
    else
      return GHom(vr,vl,this);
  }

  size_t hash() const {
    return 37;
  }

  bool operator==(const StrongHom &s) const {
    return true ; //var = ((const _plusplus &)s).var;
  }
};

/// User function : Construct a Hom for a Strong Hom _plusplus
GHom push(){return new _push();};

int main(){
  // Define a name for each variable
  initName();

  // See example from FMCAD'2002 Ciardo, Radu
  // A first evDD
  DDD ev1 = GDDD(A,0,GDDD(dA,0,GDDD(B,0,GDDD(dB,0,GDDD(C,0,GDDD(dC,0)))))) 
    + GDDD(A,0,GDDD(dA,0,GDDD(B,1,GDDD(dB,2,GDDD(C,0,GDDD(dC,0)))))) 
    + GDDD(A,1,GDDD(dA,1,GDDD(B,0,GDDD(dB,1,GDDD(C,0,GDDD(dC,0)))))) 
    + GDDD(A,1,GDDD(dA,1,GDDD(B,1,GDDD(dB,0,GDDD(C,1,GDDD(dC,0)))))) 
    + GDDD(A,2,GDDD(dA,2,GDDD(B,0,GDDD(dB,1,GDDD(C,0,GDDD(dC,0)))))) 
    + GDDD(A,2,GDDD(dA,2,GDDD(B,1,GDDD(dB,0,GDDD(C,1,GDDD(dC,0)))))) ;
  // Another evDD
  DDD ev2 = GDDD(A,0,GDDD(dA,0,GDDD(B,0,GDDD(dB,0,GDDD(C,0,GDDD(dC,0)))))) 
    + GDDD(A,0,GDDD(dA,0,GDDD(B,0,GDDD(dB,0,GDDD(C,1,GDDD(dC,2)))))) 
    + GDDD(A,1,GDDD(dA,2,GDDD(B,0,GDDD(dB,0,GDDD(C,0,GDDD(dC,0)))))) 
    + GDDD(A,1,GDDD(dA,2,GDDD(B,0,GDDD(dB,0,GDDD(C,1,GDDD(dC,2)))))) 
    + GDDD(A,2,GDDD(dA,1,GDDD(B,0,GDDD(dB,0,GDDD(C,0,GDDD(dC,0)))))) 
    + GDDD(A,2,GDDD(dA,1,GDDD(B,0,GDDD(dB,0,GDDD(C,1,GDDD(dC,2)))))) 
    + GDDD(A,2,GDDD(dA,1,GDDD(B,1,GDDD(dB,2,GDDD(C,1,GDDD(dC,0)))))) ;

  DDD ev3 = ev1 + ev2 ;
  cout << ev3;

  SDD ev1sdd = SDD(0,ev1);
  cout << ev1sdd <<endl;
  exportDot(ev1sdd,"ev1");
  SDD ev2sdd = SDD(0,ev2);
  cout << ev2sdd <<endl;
  exportDot(ev2sdd,"ev2");
  SDD ev3sdd = SDD(0,ev3);
  cout << ev3sdd <<endl;
  exportDot(ev3sdd,"ev3");

  DDD ev4 = push() (ev3);
  
  SDD ev4sdd = SDD(0,ev4);
  cout << ev4sdd <<endl;
  exportDot(ev4sdd,"ev4");
}
