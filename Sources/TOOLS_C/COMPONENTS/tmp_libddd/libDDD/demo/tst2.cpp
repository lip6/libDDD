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

int main(){
  initName();
  //  Define DDDs u, v 
  cout <<"********************"<<endl;
  cout <<"* Define DDDs u, v *"<<endl;
  cout <<"********************"<<endl;

  DDD a=DDD(A,1,DDD(A,1));
  DDD b=DDD(C,1,DDD(A,1))+DDD(C,2,DDD(B,3));
  DDD c=DDD(A,1,DDD(A,1))+DDD(A,2,DDD(B,1));
  DDD d=DDD(C,1,DDD(A,1))+DDD(C,2,DDD(B,2));
  DDD u=a^b;
  DDD v=c^d;

  cout <<"u="<< endl<<u<<endl;
  cout <<"v="<< endl<<v<<endl;

  // Identity, null, constant and  var -- val -> id
  cout <<"**************************************************"<<endl;
  cout <<"* Identity, null, constant and  var -- val -> id *"<<endl;
  cout <<"**************************************************"<<endl;

  cout <<"id(u)="<< endl<<Hom::id(u)<<endl;
  cout <<"one(u)="<< endl<<Hom(DDD::one)(u)<<endl;
  cout <<"(E-2->id)(u)="<< endl<<Hom(E,2)(u)<<endl;

  // Operations with a DDD : ^ (contenation) , * (intersection), \ (minus)
  cout <<"*************************************************************************"<<endl;
  cout <<"* Operations with a DDD : ^ (contenation) , * (intersection), \\ (minus) *"<<endl;
  cout <<"*************************************************************************"<<endl;

  cout <<"(v^id)(u)="<< endl<<(v^Hom::id)(u)<<endl;
  cout <<"(id^v)(u)="<< endl<<(Hom::id^v)(u)<<endl;
  cout <<"(v*id)(u)="<< endl<<(v*Hom::id)(u)<<endl;
  cout <<"(id\\v)(u)="<< endl<<(Hom::id-v)(u)<<endl;

  // Operations : + (union), & (composition) 
  cout <<"*******************************************"<<endl;
  cout <<"* Operations : + (union), & (composition) *"<<endl;
  cout <<"*******************************************"<<endl;

  Hom f=u^Hom::id;
  Hom g=v^Hom::id;
  cout <<"f=u^Hom::id"<<endl;
  cout <<"g=v^Hom::id"<<endl<<endl;
  cout <<"f+g(u)="<< endl<<(f+g)(u)<<endl;
  cout <<"f&g(u)="<< endl<<(f&g)(u)<<endl;
  cout <<"(E-2->id)&(f+g)(u)="<< endl<<(Hom(E,2)&(f+g))(u)<<endl;
  
  return 1;
}
