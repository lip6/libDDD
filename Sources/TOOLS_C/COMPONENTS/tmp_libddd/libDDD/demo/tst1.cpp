#include <iostream>
using namespace std;

#include "DDD.h"
#include "DED.h"
#include "MemoryManager.h"

enum {A, B, C, D,E, F, G} variables;
char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}

int main(){
  // Define a name for each variable
  initName();

  // Constants null, one , top
  cout <<"*****************************"<<endl;
  cout <<"* Constants null, one , top *"<<endl;
  cout <<"*****************************"<<endl;

  cout <<"DDD::null="<<endl<<DDD::null<<endl;
  cout <<"DDD::one="<<endl<<DDD::one<<endl;
  cout <<"DDD::top="<<endl<<DDD::top<<endl;

  // Create a DDD: var -- val -> d
  cout <<"***************"<<endl;
  cout <<"* Create DDDs *"<<endl;
  cout <<"***************"<<endl;

  DDD a(A,1);
  DDD b(A,1,DDD(B,2));
  DDD c=DDD(A,1,DDD(A,1))+DDD(A,2,DDD(B,1));
  DDD d=DDD(A,1,DDD(A,1))+DDD(A,2,DDD(B,2));
  DDD e=DDD(A,1,5,c);

  cout <<"a = A-1-><1> ="<<endl<<a<<endl;
  cout <<"b = A-1->B-2-><1> ="<<endl<<b<<endl;
  cout <<"c = A-1->A-1-><1> + A-2->B-1-><1> ="<< endl<<c<<endl;
  cout <<"d = A-1->A-1-><1> + A-2->B-2-><1>"<< endl<<d<<endl;
  cout <<"e = A-[1,5]->c ="<< endl<<e<<endl;

  // Operations + (union), * (intersection), ^ (concatenation)
  cout <<"*************************************************************"<<endl;
  cout <<"* Operations + (union), * (intersection), ^ (concatenation) *"<<endl;
  cout <<"*************************************************************"<<endl;

  cout <<"c+d="<<endl<<c+d<<endl;
  cout <<"c*d ="<<endl<<c*d<<endl;
  cout <<"c-d ="<<endl<<c-d<<endl;
  cout <<"a^c ="<<endl<<(a^c)<<endl;
  cout <<"c^a ="<<endl<<(c^a)<<endl;
  cout <<"c^d ="<<endl<<(c^d)<<endl;


  DDD f(A,1,DDD(B,1));
  DDD g(A,1,DDD(A,1));
  cout <<f+g<<endl;

  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl<<endl;

  MemoryManager::garbage();
  
  cout<<MemoryManager::nbDDD()<<endl;
  cout<<MemoryManager::nbHom()<<endl;
  cout<<MemoryManager::nbDED()<<endl;

  return 1;
}
