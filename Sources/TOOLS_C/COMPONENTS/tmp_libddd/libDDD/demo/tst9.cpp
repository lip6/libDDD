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

  DDD d1=(DDD(A,5,6) ^ DDD(B,5,7));
  DDD d2=(DDD(A,5) ^ DDD(B,6,8));

  DDD d =d1-d2;
  // [ A(5) B(5) ]
  // [ A(6) T ]

  cout<<"d1="<<d1<<endl;
  cout<<"d2="<<d2<<endl;
  cout<<"d1-d2="<<d<<endl;

  MemoryManager::pstats();


  return 1;
}
